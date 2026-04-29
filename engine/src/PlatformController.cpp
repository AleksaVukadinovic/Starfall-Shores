#include <thread>
#include <imgui_impl_glfw.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <engine/platform/PlatformController.hpp>
#include <engine/util/Utils.hpp>

#include <spdlog/spdlog.h>
#include <utility>
#include <engine/util/Configuration.hpp>

namespace engine::platform {
    static std::array<std::string_view, KEY_COUNT> g_engine_key_to_string;
    static std::array<int, KEY_COUNT> g_engine_to_glfw_key;
    static std::array<KeyId, GLFW_KEY_LAST + 1> g_glfw_key_to_engine;
    static MousePosition g_mouse_position;

    static void glfw_mouse_callback(GLFWwindow *window, double x, double y);

    static void glfw_scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

    static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height);

    static void glfw_window_close_callback(GLFWwindow *window);

    static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    static int glfw_platform_action(GLFWwindow *window, int glfw_key_code);

    void initialize_key_maps();

    void PlatformController::initialize() {
#if defined(__linux__)
        if (glfwPlatformSupported(GLFW_PLATFORM_X11)) {
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        } else if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
        }
#elif defined(_WIN32)
        if (glfwPlatformSupported(GLFW_PLATFORM_WIN32)) {
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
        }
#endif
        const bool glfw_initialized = glfwInit();
        RG_GUARANTEE(glfw_initialized, "GLFW platform failed to initialize_controllers.");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        util::Configuration::json &config = util::Configuration::config();
        const int window_width = config["window"]["width"];
        const int window_height = config["window"]["height"];
        const std::string window_title = config["window"]["title"];
        GLFWwindow *handle = glfwCreateWindow(window_width, window_height, window_title.c_str(), nullptr, nullptr);
        RG_GUARANTEE(handle, "GLFW3 platform failed to create a Window.");
        m_window = Window(handle, window_width, window_height, window_title);

        glfwMakeContextCurrent(m_window.handle_());

        int fb_width, fb_height;
        glfwGetFramebufferSize(handle, &fb_width, &fb_height);
        m_window.m_width  = fb_width;
        m_window.m_height = fb_height;
        glfwSetCursorPosCallback(m_window.handle_(), glfw_mouse_callback);
        glfwSetScrollCallback(m_window.handle_(), glfw_scroll_callback);
        glfwSetKeyCallback(m_window.handle_(), glfw_key_callback);
        glfwSetFramebufferSizeCallback(m_window.handle_(), glfw_framebuffer_size_callback);
        glfwSetMouseButtonCallback(m_window.handle_(), glfw_mouse_button_callback);
        glfwSetWindowCloseCallback(m_window.handle_(), glfw_window_close_callback);

        double init_mouse_x, init_mouse_y;
        glfwGetCursorPos(handle, &init_mouse_x, &init_mouse_y);
        g_mouse_position.x = static_cast<float>(init_mouse_x);
        g_mouse_position.y = static_cast<float>(init_mouse_y);

        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        spdlog::info(std::format("Platform[GLFW {}.{}.{}]", major, minor, revision));
        initialize_key_maps();
        m_keys.resize(KEY_COUNT);
        for (int key = 0; key < m_keys.size(); ++key) {
            m_keys[key].m_key = static_cast<KeyId>(key);
        }
    }

    void PlatformController::terminate() {
        m_platform_event_observers.clear();
        if (m_window.handle_()) {
            glfwDestroyWindow(m_window.handle_());
            glfwTerminate();
        }
    }

    bool PlatformController::loop() {
        if (m_target_fps > 0.0f) {
            const double target_frame_time = 1.0 / static_cast<double>(m_target_fps);
            while (glfwGetTime() - static_cast<double>(m_frame_time.current) < target_frame_time) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }

        m_frame_time.previous = m_frame_time.current;
        m_frame_time.current  = static_cast<float>(glfwGetTime());
        m_frame_time.dt       = m_frame_time.current - m_frame_time.previous;

        return !glfwWindowShouldClose(m_window.handle_());
    }

    void PlatformController::poll_events() {
        g_mouse_position.dx     = g_mouse_position.dy = 0.0f;
        g_mouse_position.scroll = 0.0f;
        glfwPollEvents();
        for (int i = 0; i < KEY_COUNT; ++i) {
            update_key(key_ref(static_cast<KeyId>(i)));
        }
    }

    void PlatformController::swap_buffers() const {
        glfwSwapBuffers(m_window.handle_());
    }

    int glfw_platform_action(GLFWwindow *window, const int glfw_key_code) {
        if (glfw_key_code >= GLFW_MOUSE_BUTTON_1 && glfw_key_code <= GLFW_MOUSE_BUTTON_LAST) {
            return glfwGetMouseButton(window, glfw_key_code);
        }
        return glfwGetKey(window, glfw_key_code);
    }

    /**
     * @brief Updates the state of a key. 
     * Key states are represented as a state machine with the following states: Released, JustPressed, Pressed, JustReleased.
     * The state machine transitions are as follows:
     * - Released -> JustPressed if the key is pressed.
     * - JustPressed -> Pressed if the key is still pressed.
     * - Pressed -> JustReleased if the key is released.
     * - JustReleased -> Released if the key is released.
     * @param key_data The key to update.
     */
    void PlatformController::update_key(Key &key_data) const {
        const int engine_key_code = key_data.id();
        const int glfw_key_code   = g_engine_to_glfw_key.at(engine_key_code);
        const int action          = glfw_platform_action(m_window.handle_(), glfw_key_code);
        switch (key_data.state()) {
        case Key::State::Released: {
            if (action == GLFW_PRESS) {
                key_data.m_state = Key::State::JustPressed;
            }
            break;
        }
        case Key::State::JustReleased: {
            if (action == GLFW_PRESS) {
                key_data.m_state = Key::State::JustPressed;
            } else if (action == GLFW_RELEASE) {
                key_data.m_state = Key::State::Released;
            }
            break;
        }
        case Key::State::JustPressed: {
            if (action == GLFW_RELEASE) {
                key_data.m_state = Key::State::JustReleased;
            } else if (action == GLFW_PRESS) {
                key_data.m_state = Key::State::Pressed;
            }
            break;
        }
        case Key::State::Pressed: {
            if (action == GLFW_RELEASE) {
                key_data.m_state = Key::State::JustReleased;
            }
            break;
        }
        }
    }

    std::string_view Key::name() const {
        return g_engine_key_to_string[m_key];
    }

    std::string_view Key::state_str() const {
        switch (m_state) {
        case State::Released: return "Released";
        case State::JustPressed: return "JustPressed";
        case State::Pressed: return "Pressed";
        case State::JustReleased: return "JustReleased";
        default: return "UNIMPLEMENTED";
        }
    }

    Key &PlatformController::key_ref(const KeyId key) {
        RG_GUARANTEE(key >= 0 && key < m_keys.size(), "KeyId out of bounds!");
        return m_keys[key];
    }

    const Key &PlatformController::key(const KeyId key) const {
        RG_GUARANTEE(key >= 0 && key < m_keys.size(), "KeyId out of bounds!");
        return m_keys[key];
    }

    const MousePosition &PlatformController::mouse() const {
        return g_mouse_position;
    }

    std::string_view PlatformController::name() const {
        return "PlatformGLFW3Controller";
    }

    void PlatformController::register_platform_event_observer(std::unique_ptr<PlatformEventObserver> observer) {
        m_platform_event_observers.emplace_back(std::move(observer));
    }

    void PlatformController::_platform_on_mouse(const double x, const double y) const {

        const double last_x       = g_mouse_position.x;
        const double last_y       = g_mouse_position.y;
        g_mouse_position.dx = x - last_x;
        g_mouse_position.dy = last_y - y; // because in glfw the top left corner is the (0,0)
        g_mouse_position.x  = x;
        g_mouse_position.y  = y;
        for (const auto &observer: m_platform_event_observers) {
            observer->on_mouse_move(g_mouse_position);
        }
    }

    void PlatformController::_platform_on_keyboard(int key_code, int action) const {
        const Key result = key(g_glfw_key_to_engine[key_code]);
        for (const auto &observer: m_platform_event_observers) {
            observer->on_key(result);
        }
    }

    void PlatformController::_platform_on_scroll(double x, const double y) const {
        g_mouse_position.scroll = static_cast<float>(y);
        for (const auto &observer: m_platform_event_observers) {
            observer->on_scroll(g_mouse_position);
        }
    }

    void PlatformController::_platform_on_framebuffer_resize(const int width, const int height) {
        m_window.m_width  = width;
        m_window.m_height = height;
        for (const auto &observer: m_platform_event_observers) {
            observer->on_window_resize(width, height);
        }
    }

    void PlatformController::_platform_on_window_close(GLFWwindow *window) const {
        if (m_window.handle_() == window) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    void PlatformController::_platform_on_mouse_button(const int button, int action) const {
        for (const auto &observer: m_platform_event_observers) {
            const auto result = key(g_glfw_key_to_engine[button]);
            observer->on_key(result);
        }
    }

    void PlatformController::set_enable_cursor(const bool enabled) {
        m_cursor_enabled = enabled;
        glfwSetInputMode(m_window.handle_(), GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    void PlatformController::set_fullscreen(const bool fullscreen) {
        GLFWwindow *handle = m_window.handle_();
        if (fullscreen && !m_fullscreen) {
            glfwGetWindowPos(handle, &m_windowed_x, &m_windowed_y);
            glfwGetWindowSize(handle, &m_windowed_width, &m_windowed_height);
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else if (!fullscreen && m_fullscreen) {
            glfwSetWindowMonitor(handle, nullptr, m_windowed_x, m_windowed_y, m_windowed_width, m_windowed_height, 0);
        }
        m_fullscreen = fullscreen;
    }

    void PlatformController::set_window_size(const int width, const int height) const {
        if (m_fullscreen) return;
        glfwSetWindowSize(m_window.handle_(), width, height);
    }

    bool PlatformController::is_cursor_enabled() const {
        return m_cursor_enabled;
    }

    double PlatformController::get_time() {
        return glfwGetTime();
    }

    void initialize_key_maps() {
        #include "glfw_key_mapping.include"
        #include "engine_key_to_string.include"
    }

    static void glfw_mouse_callback(GLFWwindow *window, const double x, const double y) {
        core::Controller::get<PlatformController>()->_platform_on_mouse(x, y);
    }

    void glfw_mouse_button_callback(GLFWwindow *window, const int button, const int action, int mods) {
        core::Controller::get<PlatformController>()->_platform_on_mouse_button(button, action);
    }

    static void glfw_scroll_callback(GLFWwindow *window, const double x_offset, const double y_offset) {
        g_mouse_position.scroll = static_cast<float>(y_offset);
        core::Controller::get<PlatformController>()->_platform_on_scroll(x_offset, y_offset);
    }

    static void glfw_key_callback(GLFWwindow *window, const int key, int scancode, const int action, int mods) {
        core::Controller::get<PlatformController>()->_platform_on_keyboard(key, action);
    }

    static void glfw_framebuffer_size_callback(GLFWwindow *window, const int width, const int height) {
        core::Controller::get<PlatformController>()->_platform_on_framebuffer_resize(width, height);
    }

    void glfw_window_close_callback(GLFWwindow *window) {
        core::Controller::get<PlatformController>()->_platform_on_window_close(window);
    }

} // namespace engine
