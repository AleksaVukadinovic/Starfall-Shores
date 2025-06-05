#include <imgui.h>
#include <engine/core/Engine.hpp>
#include <engine/graphics/BloomController.hpp>

#include <GUIController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <MainController.hpp>
#include <iostream>

namespace app {

    void GUIController::initialize() {
        set_enable(false);
    }

    void GUIController::poll_events() {
        if (const auto platform = get<engine::platform::PlatformController>(); platform->key(engine::platform::KeyId::KEY_G).state() == engine::platform::Key::State::JustPressed) {
            set_enable(!is_enabled());
            platform->set_enable_cursor(!platform->is_cursor_enabled());
        }
    }

    void GUIController::draw() {
        const auto graphics = get<engine::graphics::GraphicsController>();
        const auto camera   = get<engine::graphics::GraphicsController>()->camera();
        const auto bloom_controller = get<engine::graphics::BloomController>();
        const auto main_controller = get<MainController>();
        graphics->begin_gui();

        ImGui::Begin("Bloom");
        ImGui::DragFloat("Bloom Intensity", &bloom_controller->bloom_strength, 0.1f, 0.0f, 50.0f);
        ImGui::DragFloat("Exposure", &bloom_controller->exposure, 0.1f, 0.1f, 20.0f);
        ImGui::DragInt("Bloom passes", &bloom_controller->bloom_passes, 1, 0, 50);
        ImGui::End();

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
            std::cout << skybox_name << std::endl;
            main_controller->set_skybox(skybox_name, true);
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
            std::cout << skybox_name << std::endl;
            main_controller->set_skybox(skybox_name, false);
        }
        ImGui::End();

        ImGui::Begin("Camera info");
        const auto &c = *camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::End();
        graphics->end_gui();
    }
}
