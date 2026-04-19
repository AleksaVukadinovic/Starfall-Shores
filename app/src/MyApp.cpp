#include <GUIController.hpp>
#include <MainController.hpp>
#include <MyApp.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <spdlog/spdlog.h>

namespace app {
    void MyApp::app_setup() {
        spdlog::info("App setup completed");
        const auto main_controller = register_controller<MainController>();
        const auto gui_controller  = register_controller<GUIController>();
        const auto post_processing_controller = register_controller<engine::graphics::PostProcessingController>();
        const auto fog_controller = register_controller<FogController>();
        const auto greyscale_controller = register_controller<GreyscaleController>();
        const auto rain_controller = register_controller<RainController>();
        const auto fps_camera = register_controller<engine::graphics::FPSCameraController>();
        const auto shadow_controller = register_controller<engine::graphics::ShadowController>();
        const auto lighting_controller = register_controller<engine::graphics::LightingController>();
        const auto renderer = register_controller<engine::graphics::Renderer>();
        main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        fps_camera->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        shadow_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        lighting_controller->after(shadow_controller);
        renderer->after(lighting_controller);
        post_processing_controller->after(renderer);
        fog_controller->after(post_processing_controller);
        greyscale_controller->after(post_processing_controller);
        rain_controller->after(post_processing_controller);
        main_controller->after(post_processing_controller);
        gui_controller->after(main_controller);
    }

}