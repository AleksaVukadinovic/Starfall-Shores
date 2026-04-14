#include <GUIController.hpp>
#include <MainController.hpp>
#include <MyApp.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <spdlog/spdlog.h>

namespace app {
    void MyApp::app_setup() {
        spdlog::info("App setup completed");
        const auto main_controller = register_controller<MainController>();
        const auto gui_controller  = register_controller<GUIController>();
        const auto bloom_controller = register_controller<engine::graphics::PostProcessingController>();
        const auto fog_controller = register_controller<FogController>();
        const auto greyscale_controller = register_controller<GreyscaleController>();
        main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        bloom_controller->after(main_controller);
        fog_controller->after(main_controller);
        greyscale_controller->after(main_controller);
        gui_controller->after(main_controller);
        bloom_controller->after(gui_controller);
    }

}