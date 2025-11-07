/**
 * @file PlatformController.hpp
 * @brief Defines the PlatformController class that serves as the interface for platform-specific functionality, such as input handling and window management.
 */

#ifndef MATF_RG_PROJECT_PLATFORM_H
#define MATF_RG_PROJECT_PLATFORM_H

#include <engine/core/Controller.hpp>
#include <memory>
#include <vector>
#include <engine/platform/Input.hpp>
#include <engine/platform/Window.hpp>
#include <engine/platform/PlatformEventObserver.hpp>

struct GLFWwindow;

namespace engine::platform {
    /**
    * @struct FrameTime
    * @brief Stores elapsed time for frames in seconds.
    */
    struct FrameTime {
        /**
        * @brief Elapsed seconds for the previous frame.
        */
        float dt;

        /**
        * @brief Time from the initialization of the Platform to the moment when the previous frame began.
        */
        float previous;

        /**
        * @brief Time from the initialization of the Platform to the moment when the current frame began.
        */
        float current;
    };

    /**
    * @class PlatformController
    * @brief Registers Platform events such as mouse movement, key press, window events...
    *
    */
    class PlatformController final : public core::Controller {
    public:
        /**
        * @brief Get the state of the @ref Key in the current frame
        * @param key An @ref KeyId for the key
        * @returns The state of the key in the current frame
        */
        const Key &key(KeyId key) const;

        /**
        * @brief Get the state of the @ref MousePosition in the current frame
        * @returns @ref MousePosition for the current frame.
        */
        const MousePosition &mouse() const;

        /**
        * @brief Get the name of the Controller
        * @returns "PlatformController"
        */
        std::string_view name() const override;

        /**
        * @brief Get the window
        * @returns @ref Window
        */
        const Window *window() const {
            return &m_window;
        }

        /**
        * @brief Register a @ref PlatformEventObserver callback for platform events.
        * By default, the @ref PlatformController registers a @ref PlatformEventObserver that does nothing.
        */
        void register_platform_event_observer(std::unique_ptr<PlatformEventObserver> observer);

        /**
        * @brief Get @ref FrameTime for the previous frame. Updated in during @ref core::App::loop
        * @returns @ref FrameTime
        */
        const FrameTime &frame_time() const {
            return m_frame_time;
        }

        /**
        * @brief Get elapsed time for the previous frame.
        *
        */
        float dt() const {
            return m_frame_time.dt;
        }

        /**
        *  @brief Enables/disabled the visibility of the cursor on screen.
        */
        void set_enable_cursor(bool enabled);

        /**
        * @brief Check if the cursor is currently enabled/visible.
        * @returns true if cursor is enabled, false otherwise
        */
        [[nodiscard]] bool is_cursor_enabled() const;

        [[nodiscard]] static double get_time();

        /**
        * @brief Swaps the current draw buffer for the main window. Should be called at the end of the frame.
        */
        void swap_buffers();

        /**
        * @brief Called from the platform-specific callback. You shouldn't call this function directly.
        */
        void _platform_on_mouse(double x, double y);

        /**
        * @brief Called from the platform-specific callback. You shouldn't call this function directly.
        */
        void _platform_on_keyboard(int key, int action);

        /**
        * @brief Called from the platform-specific callback. You shouldn't call this function directly.
        */
        void _platform_on_scroll(double x, double y);

        /**
        * @brief Called from the platform-specific callback. You shouldn't call this function directly.
        */
        void _platform_on_framebuffer_resize(int width, int height);

        /**
        * @brief Called from the platform-specific callback. You shouldn't call this function directly.
        */
        void _platform_on_window_close(GLFWwindow *window);

        void _platform_on_mouse_button(int button, int action);

    private:
        Key &key_ref(KeyId key);

        /**
        * @brief Initializes the platform layer and registers platform-specific event callbacks.
        */
        void initialize() override;

        /**
        * @brief Terminate the platform layer.
        */
        void terminate() override;

        bool loop() override;

        void poll_events() override;

        void update_mouse();

        void update_key(Key &key_data) const;

        FrameTime m_frame_time;
        Window m_window;
        std::vector<Key> m_keys;
        std::vector<std::unique_ptr<PlatformEventObserver> > m_platform_event_observers;

        /**
        * @brief Tracks whether the cursor is currently enabled/visible
        */
        bool m_cursor_enabled = true;
    };
} // namespace engine

#endif//MATF_RG_PROJECT_PLATFORM_H
