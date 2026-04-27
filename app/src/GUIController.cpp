#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/platform/TimeController.hpp>
#include <imgui.h>

namespace app {

static const ImVec4 COL_BG         = {0.08f, 0.08f, 0.10f, 0.95f};
static const ImVec4 COL_PANEL      = {0.12f, 0.12f, 0.15f, 1.00f};
static const ImVec4 COL_ACCENT     = {0.35f, 0.55f, 0.95f, 1.00f};
static const ImVec4 COL_ACCENT_HOV = {0.45f, 0.65f, 1.00f, 1.00f};
static const ImVec4 COL_ACCENT_ACT = {0.25f, 0.45f, 0.85f, 1.00f};
static const ImVec4 COL_TEXT       = {0.90f, 0.90f, 0.92f, 1.00f};
static const ImVec4 COL_TEXT_DIM   = {0.55f, 0.55f, 0.58f, 1.00f};
static const ImVec4 COL_SIDEBAR    = {0.10f, 0.10f, 0.13f, 1.00f};
static const ImVec4 COL_SIDEBAR_HL = {0.18f, 0.18f, 0.22f, 1.00f};
static const ImVec4 COL_SLIDER_BG  = {0.18f, 0.18f, 0.22f, 1.00f};
static const ImVec4 COL_SLIDER_GRB = {0.35f, 0.55f, 0.95f, 0.80f};
static const ImVec4 COL_CHECK      = {0.35f, 0.55f, 0.95f, 1.00f};
static const ImVec4 COL_QUIT       = {0.85f, 0.25f, 0.25f, 1.00f};
static const ImVec4 COL_QUIT_HOV   = {0.95f, 0.35f, 0.35f, 1.00f};
static const ImVec4 COL_QUIT_ACT   = {0.75f, 0.15f, 0.15f, 1.00f};

void GUIController::push_style() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,    8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,     6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding,      6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,     6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,      ImVec2(10.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,       ImVec2(10.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,     ImVec2(20.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,     12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,       10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,  0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,   0.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,         COL_BG);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,          COL_PANEL);
    ImGui::PushStyleColor(ImGuiCol_Text,             COL_TEXT);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled,     COL_TEXT_DIM);
    ImGui::PushStyleColor(ImGuiCol_Button,           COL_ACCENT);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,    COL_ACCENT_HOV);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,     COL_ACCENT_ACT);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          COL_SLIDER_BG);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   {0.22f, 0.22f, 0.28f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    {0.25f, 0.25f, 0.32f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       COL_SLIDER_GRB);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, COL_ACCENT);
    ImGui::PushStyleColor(ImGuiCol_CheckMark,        COL_CHECK);
    ImGui::PushStyleColor(ImGuiCol_Header,           COL_SIDEBAR_HL);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,    {0.22f, 0.22f, 0.28f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,     COL_ACCENT_ACT);
    ImGui::PushStyleColor(ImGuiCol_Separator,        {0.22f, 0.22f, 0.28f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,      {0.10f, 0.10f, 0.13f, 0.5f});
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,    {0.30f, 0.30f, 0.35f, 1.0f});
}

void GUIController::pop_style() {
    ImGui::PopStyleColor(19);
    ImGui::PopStyleVar(12);
}

void GUIController::initialize() {
    m_graphics = get<engine::graphics::GraphicsController>();
    m_camera = m_graphics->camera();
    m_bloom = get<engine::graphics::PostProcessingController>();
    m_platform = get<engine::platform::PlatformController>();
    m_fog = get<FogController>();
    m_greyscale = get<GreyscaleController>();
    m_rain = get<RainController>();
    m_main_controller = get<MainController>();
    m_fps_camera = get<engine::graphics::FPSCameraController>();
    m_shadow = get<engine::graphics::ShadowController>();
    m_time = get<engine::platform::TimeController>();
    m_fov = m_graphics->perspective_params().FOV;
}

void GUIController::poll_events() {
    if (const auto platform = get<engine::platform::PlatformController>();
        platform->key(engine::platform::KeyId::KEY_ESCAPE).state() == engine::platform::Key::State::JustPressed) {
        if (m_menu_open) {
            if (m_page == MenuPage::Settings) {
                m_page = MenuPage::Main;
            } else {
                m_menu_open = false;
                platform->set_enable_cursor(false);
                m_fps_camera->set_enable(true);
            }
        } else {
            m_page = MenuPage::Main;
            m_menu_open = true;
            platform->set_enable_cursor(true);
            m_fps_camera->set_enable(false);
        }
    }
}

void GUIController::draw() {
    m_graphics->begin_gui();

    if (m_menu_open) {
        push_style();

        m_fps_accumulator += m_platform->dt();
        m_fps_frame_count++;
        if (m_fps_accumulator >= 0.5f) {
            m_last_recorded_fps = static_cast<float>(m_fps_frame_count) / m_fps_accumulator;
            m_fps_accumulator = 0.0f;
            m_fps_frame_count = 0;
        }

        switch (m_page) {
            case MenuPage::Main:    draw_main_menu(); break;
            case MenuPage::Settings: draw_settings();  break;
        }

        pop_style();
    }

    m_graphics->end_gui();
}

void GUIController::draw_main_menu() {
    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    constexpr float menu_w = 320.0f;
    constexpr float menu_h = 300.0f;

    ImGui::SetNextWindowPos({center.x - menu_w * 0.5f, center.y - menu_h * 0.5f});
    ImGui::SetNextWindowSize({menu_w, menu_h});
    ImGui::Begin("##PauseMenu", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoCollapse);

    ImGui::Dummy({0, 10});

    float title_w = ImGui::CalcTextSize("PAUSED").x;
    ImGui::SetCursorPosX((menu_w - title_w) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, COL_TEXT);
    ImGui::Text("PAUSED");
    ImGui::PopStyleColor();

    ImGui::Dummy({0, 8});
    ImGui::Separator();
    ImGui::Dummy({0, 16});

    constexpr float btn_w = 220.0f;
    constexpr float btn_h = 40.0f;
    const float btn_x = (menu_w - btn_w) * 0.5f;

    ImGui::SetCursorPosX(btn_x);
    if (ImGui::Button("Resume", {btn_w, btn_h})) {
        m_menu_open = false;
        m_platform->set_enable_cursor(false);
        m_fps_camera->set_enable(true);
    }

    ImGui::Dummy({0, 4});
    ImGui::SetCursorPosX(btn_x);
    if (ImGui::Button("Settings", {btn_w, btn_h})) {
        m_page = MenuPage::Settings;
        m_settings_category = SettingsCategory::Graphics;
    }

    ImGui::Dummy({0, 4});
    ImGui::SetCursorPosX(btn_x);
    ImGui::PushStyleColor(ImGuiCol_Button,        COL_QUIT);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  COL_QUIT_HOV);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   COL_QUIT_ACT);
    if (ImGui::Button("Quit", {btn_w, btn_h})) {
        m_wants_quit = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::End();
}

void GUIController::draw_settings() {
    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    constexpr float win_w = 700.0f;
    constexpr float win_h = 480.0f;

    ImGui::SetNextWindowPos({center.x - win_w * 0.5f, center.y - win_h * 0.5f});
    ImGui::SetNextWindowSize({win_w, win_h});
    ImGui::Begin("##Settings", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleColor(ImGuiCol_Text, COL_TEXT_DIM);
    if (ImGui::Button("< Back")) {
        m_page = MenuPage::Main;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    float title_w = ImGui::CalcTextSize("SETTINGS").x;
    ImGui::SetCursorPosX((win_w - title_w) * 0.5f);
    ImGui::Text("SETTINGS");

    ImGui::Dummy({0, 4});
    ImGui::Separator();
    ImGui::Dummy({0, 8});

    ImGui::BeginChild("##sidebar", {170, 0}, true, ImGuiWindowFlags_NoScrollbar);
    draw_settings_sidebar();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##content", {0, 0}, true);
    draw_settings_content();
    ImGui::EndChild();

    ImGui::End();
}

void GUIController::draw_settings_sidebar() {
    constexpr float btn_w = 150.0f;
    constexpr float btn_h = 34.0f;

    auto sidebar_button = [&](const char *label, SettingsCategory cat) {
        const bool selected = m_settings_category == cat;
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, COL_ACCENT);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_ACCENT);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, COL_SIDEBAR);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_SIDEBAR_HL);
        }
        if (ImGui::Button(label, {btn_w, btn_h})) {
            m_settings_category = cat;
        }
        ImGui::PopStyleColor(2);
    };

    ImGui::Dummy({0, 4});
    sidebar_button("Graphics",    SettingsCategory::Graphics);
    sidebar_button("Environment", SettingsCategory::Environment);
    sidebar_button("Sound",       SettingsCategory::Sound);
    sidebar_button("Time",        SettingsCategory::Time);
    sidebar_button("Debug",       SettingsCategory::Debug);
}

void GUIController::draw_settings_content() {
    switch (m_settings_category) {
        case SettingsCategory::Graphics:    draw_graphics_settings();     break;
        case SettingsCategory::Environment: draw_environment_settings();  break;
        case SettingsCategory::Sound:       draw_sound_settings();        break;
        case SettingsCategory::Time:        draw_time_settings();         break;
        case SettingsCategory::Debug:       draw_debug_settings();        break;
    }
}

static void section_header(const char *label) {
    ImGui::PushStyleColor(ImGuiCol_Text, COL_ACCENT);
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Dummy({0, 4});
}

void GUIController::draw_graphics_settings() const {
    section_header("Bloom");
    ImGui::SliderFloat("Bloom Intensity", &m_bloom->bloom_strength, 0.0f, 50.0f);
    ImGui::SliderFloat("Exposure",        &m_bloom->exposure, 0.1f, 20.0f);
    int passes = static_cast<int>(m_bloom->bloom_passes);
    if (ImGui::SliderInt("Bloom Passes", &passes, 0, 20))
        m_bloom->bloom_passes = static_cast<uint32_t>(passes);

    ImGui::Dummy({0, 8});
    section_header("Shadows");
    ImGui::Checkbox("Enable Shadows", &m_shadow->enabled);

    ImGui::Dummy({0, 8});
    section_header("Post-Processing");
    ImGui::Checkbox("Enable Greyscale",    &m_greyscale->greyscale_enabled);
    if (m_greyscale->greyscale_enabled)
        ImGui::SliderFloat("Greyscale Strength", &m_greyscale->greyscale_strength, 0.0f, 1.0f);

    ImGui::Dummy({0, 8});
    section_header("Camera");
    float fov = m_graphics->perspective_params().FOV;
    if (ImGui::SliderFloat("Field of View", &fov, 0.1f, 2.0f))
        m_graphics->perspective_params().FOV = fov;

    static const char *fps_options[] = {"Unlimited", "30", "60", "120", "144", "240"};
    static constexpr float fps_values[] = {0.0f, 30.0f, 60.0f, 120.0f, 144.0f, 240.0f};
    int current = 0;
    const float target = m_platform->target_fps();
    for (int i = 0; i < 6; ++i) {
        if (fps_values[i] == target) { current = i; break; }
    }
    if (ImGui::Combo("FPS Limit", &current, fps_options, 6))
        m_platform->set_target_fps(fps_values[current]);

    ImGui::TextDisabled("FPS: %.0f", m_last_recorded_fps);
}

void GUIController::draw_environment_settings() const {
    section_header("Fog");
    ImGui::Checkbox("Enable Fog", &m_fog->fog_enabled);
    if (m_fog->fog_enabled) {
        ImGui::SliderFloat("Fog Intensity", &m_fog->fog_intensity, 0.1f, 10.0f);
        ImGui::SliderFloat("Fog Start",     &m_fog->fog_start, 0.0f, 500.0f);
        ImGui::SliderFloat("Fog End",       &m_fog->fog_end, 0.0f, 500.0f);
    }

    ImGui::Dummy({0, 8});
    section_header("Rain");
    ImGui::Checkbox("Enable Rain", &m_rain->rain_enabled);
    if (m_rain->rain_enabled) {
        ImGui::SliderFloat("Rain Intensity",  &m_rain->rain_intensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Rain Speed",      &m_rain->rain_speed, 0.1f, 5.0f);
        ImGui::SliderFloat("Rain Opacity",    &m_rain->rain_opacity, 0.0f, 1.0f);
        ImGui::SliderFloat("Streak Length",   &m_rain->rain_streak_length, 0.02f, 0.5f);
    }

    ImGui::Dummy({0, 8});
    section_header("Wind");
    auto renderer = engine::core::Controller::get<engine::graphics::Renderer>();
    ImGui::Checkbox("Enable Wind", &renderer->wind.enabled);
    if (renderer->wind.enabled)
        ImGui::SliderFloat("Wind Intensity", &renderer->wind.intensity, 0.0f, 1.0f);

    ImGui::Dummy({0, 8});
    section_header("Skybox");
    const char *daytime_skyboxes[] = {"Sunny", "Cloudy", "Islands"};
    static int current_day = 0;
    if (ImGui::Combo("Daytime Skybox", &current_day, daytime_skyboxes, 3)) {
        const char *names[] = {"skybox_day2", "skybox_day", "skybox_default"};
        m_main_controller->set_skybox(names[current_day], true);
    }
    const char *nighttime_skyboxes[] = {"Night Canyon", "Night Stars", "Night Dark"};
    static int current_night = 0;
    if (ImGui::Combo("Nighttime Skybox", &current_night, nighttime_skyboxes, 3)) {
        const char *names[] = {"skybox_night", "skybox_night_stars", "skybox_night_dark"};
        m_main_controller->set_skybox(names[current_night], false);
    }
}

void GUIController::draw_sound_settings() const {
    section_header("Audio");
    ImGui::Checkbox("Enable Sound",  &m_main_controller->m_sound_enabled);
    ImGui::SliderFloat("Volume", &m_main_controller->m_sound_volume, 0.0f, 150.0f, "%.0f%%");
}

void GUIController::draw_time_settings() const {
    section_header("In-Game Time");
    ImGui::Text("Current Time: %s", m_time->formatted_time().c_str());
    ImGui::Text("%s", m_time->is_night() ? "Night" : "Day");

    ImGui::Dummy({0, 4});
    bool running = m_time->is_running();
    if (ImGui::Checkbox("Time Running", &running))
        m_time->set_running(running);
    float tickrate = m_time->tickrate();
    if (ImGui::SliderFloat("Time Speed", &tickrate, 0.0f, 10.0f))
        m_time->set_tickrate(tickrate);
}

void GUIController::draw_debug_settings() const {
    section_header("Camera");
    const auto &c = *m_camera;
    ImGui::Text("Position: (%.1f, %.1f, %.1f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("Yaw: %.1f  Pitch: %.1f", c.Yaw, c.Pitch);
    ImGui::Text("Front: (%.2f, %.2f, %.2f)", c.Front.x, c.Front.y, c.Front.z);

    ImGui::Dummy({0, 8});
    section_header("Model Placement");
    ImGui::Text("Select Model:");
    for (int i = 0; i < static_cast<int>(MainController::TEST_MODEL_NAMES.size()); ++i) {
        ImGui::RadioButton(MainController::TEST_MODEL_NAMES[i], &m_main_controller->selected_model_index, i);
        if (i % 3 != 2 && i + 1 < static_cast<int>(MainController::TEST_MODEL_NAMES.size()))
            ImGui::SameLine();
    }

    ImGui::Dummy({0, 4});
    ImGui::SliderFloat("Translate X", &m_main_controller->test_translation.x, -200.0f, 200.0f);
    ImGui::SliderFloat("Translate Y", &m_main_controller->test_translation.y, -200.0f, 200.0f);
    ImGui::SliderFloat("Translate Z", &m_main_controller->test_translation.z, -200.0f, 200.0f);
    ImGui::InputFloat("Rotate X", &m_main_controller->test_rotation.x, -1.0f, 5.0f);
    ImGui::InputFloat("Rotate Y", &m_main_controller->test_rotation.y, -1.0f, 5.0f);
    ImGui::InputFloat("Rotate Z", &m_main_controller->test_rotation.z, -1.0f, 5.0f);
    ImGui::InputFloat("Scale",    &m_main_controller->test_scale, 0.001f, 50.0f);

    ImGui::Dummy({0, 4});
    if (ImGui::Button("Place Model"))
        m_main_controller->place_test_model();
    ImGui::SameLine();
    if (ImGui::Button("Clear All"))
        m_main_controller->clear_placed_models();
    ImGui::Text("Placed: %zu", m_main_controller->placed_models.size());
}

} // namespace app