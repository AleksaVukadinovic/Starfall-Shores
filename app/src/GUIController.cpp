#include <engine/core/Engine.hpp>
#include <engine/graphics/BloomController.hpp>
#include <engine/graphics/FogController.hpp>
#include <imgui.h>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/graphics/GraphicsController.hpp>

namespace app {

void GUIController::initialize() {
    set_enable(false);
    m_graphics = get<engine::graphics::GraphicsController>();
    m_camera = get<engine::graphics::GraphicsController>()->camera();
    m_bloom = get<engine::graphics::BloomController>();
    m_platform = get<engine::platform::PlatformController>();
    m_fog = get<FogController>();
    m_main_controller = get<MainController>();
    m_current_time = m_platform->get_time();
    m_fov = m_graphics->perspective_params().FOV;
}

void GUIController::poll_events() {
    if (const auto platform = get<engine::platform::PlatformController>(); platform->key(engine::platform::KeyId::KEY_G).state() == engine::platform::Key::State::JustPressed) {
        set_enable(!is_enabled());
        platform->set_enable_cursor(!platform->is_cursor_enabled());
    }
}

void GUIController::draw() {
    m_graphics->begin_gui();
    draw_bloom_controls();
    draw_fog_controls();
    draw_skybox_controls();
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
        spdlog::info("{}", skybox_name);
        m_main_controller->set_skybox(skybox_name, true);
    }

    const char* nighttime_skyboxes[] = { "Night Canyon", "Night Stars" };
    static int current_nighttime_skybox = 0;
    if (ImGui::Combo("Nighttime Skybox", &current_nighttime_skybox, nighttime_skyboxes, IM_ARRAYSIZE(nighttime_skyboxes))) {
        std::string skybox_name;
        switch (current_nighttime_skybox) {
            case 0: skybox_name = "skybox_night"; break;
            case 1: skybox_name = "skybox_night_stars"; break;
            default: skybox_name = "skybox_night"; break;
        }
        spdlog::info("{}", skybox_name);
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
    m_current_time = m_platform->get_time();
    if (m_tickrate > 0.0f && m_last_update_time + 1.0f/m_tickrate >= m_current_time) {
        ImGui::Text("%.0f", m_last_recorded_fps);
    } else {
        m_last_update_time = m_current_time;
        m_last_recorded_fps = 1.0f/m_platform->dt();
        ImGui::Text("%.0f", m_last_recorded_fps);
    }
    ImGui::End();
}

void GUIController::draw_tickrate_slider() {
    ImGui::Begin("Select tickrate");
    ImGui::SliderFloat("Select tickrate", &m_tickrate, 0.1f, 10.0f);
    ImGui::End();
}

void GUIController::draw_fov_slider() {
    ImGui::Begin("Select FOV");
    ImGui::SliderFloat("Select FOV", &m_fov, 1.0f, 50.0f);
    m_graphics->perspective_params().FOV = m_fov;
    ImGui::End();
}

}// namespace app
