#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/RainController.hpp>
#include <imgui.h>

namespace app {

void GUIController::initialize() {
    set_enable(false);
    m_graphics = get<engine::graphics::GraphicsController>();
    m_camera = get<engine::graphics::GraphicsController>()->camera();
    m_bloom = get<engine::graphics::PostProcessingController>();
    m_platform = get<engine::platform::PlatformController>();
    m_fog = get<FogController>();
    m_greyscale = get<GreyscaleController>();
    m_rain = get<RainController>();
    m_main_controller = get<MainController>();
    m_fps_camera = get<engine::graphics::FPSCameraController>();
    m_shadow = get<engine::graphics::ShadowController>();
    m_last_update_time = static_cast<float>(engine::platform::PlatformController::get_time());
    m_fov = m_graphics->perspective_params().FOV;
}

void GUIController::poll_events() {
    if (const auto platform = get<engine::platform::PlatformController>(); platform->key(engine::platform::KeyId::KEY_G).state() == engine::platform::Key::State::JustPressed) {
        const bool opening = !is_enabled();
        set_enable(opening);
        platform->set_enable_cursor(opening);
        m_fps_camera->set_enable(!opening);
    }
}

void GUIController::draw() {
    m_graphics->begin_gui();
    draw_bloom_controls();
    draw_fog_controls();
    draw_greyscale_controls();
    draw_rain_controls();
    draw_wind_controls();
    draw_shadow_controls();
    draw_skybox_controls();
    draw_test_controls();
    draw_camera_info();
    draw_fov_slider();
    draw_tickrate_slider();
    draw_fps_counter();
    m_graphics->end_gui();
}

void GUIController::draw_bloom_controls() const {
    ImGui::Begin("Bloom");
    ImGui::DragFloat("Bloom Intensity", &m_bloom->bloom_strength, 0.1f, 0.0f, 50.0f);
    ImGui::DragFloat("Exposure", &m_bloom->exposure, 0.1f, 0.1f, 20.0f);
    ImGui::DragScalar("Bloom passes", ImGuiDataType_U32, &m_bloom->bloom_passes, 1.0f);
    ImGui::End();
}

void GUIController::draw_fog_controls() const {
    ImGui::Begin("Fog");
    ImGui::Checkbox("Enable Fog", &m_fog->fog_enabled);
    ImGui::SliderFloat("Fog Intensity", &m_fog->fog_intensity, 0.1f, 10.0f);
    ImGui::SliderFloat("Fog Start", &m_fog->fog_start, 0.0f, 500.0f);
    ImGui::SliderFloat("Fog End", &m_fog->fog_end, 0.0f, 500.0f);
    ImGui::End();
}

void GUIController::draw_skybox_controls() const {
    ImGui::Begin("Skybox Selection");
    const char* daytime_skyboxes[] = { "Sunny", "Cloudy", "Islands" };
    static int current_daytime_skybox = 0;
    if (ImGui::Combo("Daytime Skybox", &current_daytime_skybox, daytime_skyboxes, IM_ARRAYSIZE(daytime_skyboxes))) {
        std::string skybox_name;
        switch (current_daytime_skybox) {
            case 0: skybox_name = "skybox_day2"; break;
            case 1: skybox_name = "skybox_day"; break;
            case 2: skybox_name = "skybox_default"; break;
            default: skybox_name = "skybox_day"; break;
        }
        m_main_controller->set_skybox(skybox_name, true);
    }

    const char* nighttime_skyboxes[] = { "Night Canyon", "Night Stars", "Night Dark" };
    static int current_nighttime_skybox = 0;
    if (ImGui::Combo("Nighttime Skybox", &current_nighttime_skybox, nighttime_skyboxes, IM_ARRAYSIZE(nighttime_skyboxes))) {
        std::string skybox_name;
        switch (current_nighttime_skybox) {
            case 0: skybox_name = "skybox_night"; break;
            case 1: skybox_name = "skybox_night_stars"; break;
            case 2: skybox_name = "skybox_night_dark"; break;
            default: skybox_name = "skybox_night"; break;
        }
        spdlog::info(std::format("{}", skybox_name));
        m_main_controller->set_skybox(skybox_name, false);
    }
    ImGui::End();
}

void GUIController::draw_camera_info() const {
    ImGui::Begin("Camera info");
    const auto &c = *m_camera;
    ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    ImGui::End();
}

void GUIController::draw_fps_counter() {
    ImGui::Begin("FPS");
    m_fps_accumulator += m_platform->dt();
    m_fps_frame_count++;
    if (const float update_interval = m_tickrate > 0.0f ? 1.0f / m_tickrate : 0.5f; m_fps_accumulator >= update_interval) {
        m_last_recorded_fps = static_cast<float>(m_fps_frame_count) / m_fps_accumulator;
        m_fps_accumulator = 0.0f;
        m_fps_frame_count = 0;
    }
    static const char* fps_options[] = { "Unlimited", "30", "60", "120", "144", "240" };
    static constexpr float fps_values[] = { 0.0f, 30.0f, 60.0f, 120.0f, 144.0f, 240.0f };
    int current = 0;
    const float target = m_platform->target_fps();
    for (int i = 0; i < sizeof(fps_values)/sizeof(int); ++i) {
        if (fps_values[i] == target) { current = i; break; }
    }
    if (ImGui::Combo("FPS Limit", &current, fps_options, 6)) {
        m_platform->set_target_fps(fps_values[current]);
    }
    ImGui::Text("%.0f", m_last_recorded_fps);
    ImGui::End();
}

void GUIController::draw_tickrate_slider() {
    ImGui::Begin("Select tickrate");
    ImGui::SliderFloat("Select tickrate", &m_tickrate, 0.1f, 10.0f);
    ImGui::End();
}

void GUIController::draw_fov_slider() {
    ImGui::Begin("Select FOV");
    ImGui::SliderFloat("Select FOV", &m_fov, 0.1f, 2.0f);
    m_graphics->perspective_params().FOV = m_fov;
    ImGui::End();
}

void GUIController::draw_greyscale_controls() const {
    ImGui::Begin("Greyscale");
    ImGui::Checkbox("Enable Greyscale", &m_greyscale->greyscale_enabled);
    ImGui::SliderFloat("Greyscale Strength", &m_greyscale->greyscale_strength, 0.0f, 1.0f);
    ImGui::End();
}

void GUIController::draw_rain_controls() const {
    ImGui::Begin("Rain");
    ImGui::Checkbox("Enable Rain (R)", &m_rain->rain_enabled);
    ImGui::SliderFloat("Rain Intensity", &m_rain->rain_intensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Rain Speed", &m_rain->rain_speed, 0.1f, 5.0f);
    ImGui::SliderFloat("Rain Opacity", &m_rain->rain_opacity, 0.0f, 1.0f);
    ImGui::SliderFloat("Streak Length", &m_rain->rain_streak_length, 0.02f, 0.5f);
    ImGui::End();
}

void GUIController::draw_wind_controls() const {
    ImGui::Begin("Wind");
    ImGui::Checkbox("Enable Wind (V)", &m_main_controller->wind_enabled);
    ImGui::SliderFloat("Wind Intensity", &m_main_controller->wind_intensity, 0.0f, 1.0f);
    ImGui::End();
}

void GUIController::draw_shadow_controls() const {
    ImGui::Begin("Shadows");
    ImGui::Checkbox("Enable Shadows", &m_shadow->enabled);
    ImGui::End();
}

void GUIController::draw_test_controls() const {
    ImGui::Begin("Model Test");

    ImGui::Text("Select Model:");
    for (uint8_t i = 0; i < static_cast<uint8_t>(MainController::TEST_MODEL_NAMES.size()); ++i) {
        ImGui::RadioButton(MainController::TEST_MODEL_NAMES[i], &m_main_controller->selected_model_index, i);
        if (i % 3 != 2 && i + 1 < static_cast<uint8_t>(MainController::TEST_MODEL_NAMES.size()))
            ImGui::SameLine();
    }

    ImGui::Separator();
    ImGui::SliderFloat("Translate X", &m_main_controller->test_translation.x, -200.0f, 200.0f);
    ImGui::SliderFloat("Translate Y", &m_main_controller->test_translation.y, -200.0f, 200.0f);
    ImGui::SliderFloat("Translate Z", &m_main_controller->test_translation.z, -200.0f, 200.0f);
    ImGui::InputFloat("Rotate X", &m_main_controller->test_rotation.x, -1.0f, 5.0f);
    ImGui::InputFloat("Rotate Y", &m_main_controller->test_rotation.y, -1.0f, 5.0f);
    ImGui::InputFloat("Rotate Z", &m_main_controller->test_rotation.z, -1.0f, 5.0f);
    ImGui::InputFloat("Scale", &m_main_controller->test_scale, 0.001f, 50.0f);

    ImGui::Separator();
    if (ImGui::Button("Place Model"))
        m_main_controller->place_test_model();
    ImGui::SameLine();
    if (ImGui::Button("Clear All Placed"))
        m_main_controller->clear_placed_models();
    ImGui::Text("Placed models: %zu", m_main_controller->placed_models.size());
    ImGui::End();
}

}// namespace app
