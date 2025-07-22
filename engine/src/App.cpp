#include <spdlog/spdlog.h>
#include <engine/core/App.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Errors.hpp>

#include <engine/util/ArgParser.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/util/Utils.hpp>

namespace engine::core {
int App::run(int argc, char **argv) {
    try {
        engine_setup(argc, argv);
        app_setup();
        initialize();
        while (loop()) {
            poll_events();
            update();
            draw();
        }
        terminate();
    } catch (const util::Error &e) {
        handle_error(e);
        terminate();
    }
    return on_exit();
}

void App::engine_setup(int argc, char **argv) {
    util::ArgParser::instance()->initialize(argc, argv);
    util::Configuration::instance()->initialize();

    // register engine controllers
    auto begin = register_controller<EngineControllersBegin>();
    auto platform = register_controller<platform::PlatformController>();
    auto graphics = register_controller<graphics::GraphicsController>();
    auto resources = register_controller<resources::ResourcesController>();
    auto end = register_controller<EngineControllersEnd>();
    begin->before(platform);
    platform->before(graphics);
    graphics->before(resources);
    resources->before(end);
}

void App::initialize() {
    // Topologically sort controllers based on their dependency graph formed by before/after methods.
    {
        auto adjacent_controllers = [](Controller *curr) {
            return curr->next();
        };
        RG_GUARANTEE(!util::alg::has_cycle(range(m_controllers), adjacent_controllers),
                     "Please make sure that there are no cycles in the controller dependency graph.");
        util::alg::topological_sort(range(m_controllers), adjacent_controllers);
    }
    for (auto controller: m_controllers) {
        spdlog::info("{}::initialize", controller->name());
        controller->initialize();
    }
}

bool App::loop() {
    for (auto controller: m_controllers) {
        if (controller->is_enabled() && !controller->loop()) {
            return false;
        }
    }
    return true;
}

void App::poll_events() {
    for (auto controller: m_controllers) {
        // We don't check if the controller is enabled for poll_events because the controller may enable itself in the poll_events if it needs to.
        // For example, a GUIController may enable itself in the poll_events method if a button to enable/disable the GUI was pressed.
        controller->poll_events();
    }
}

void App::update() {
    for (auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->update();
        }
    }
}

void App::draw() {
    for (auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->begin_draw();
        }
    }
    for (auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->draw();
        }
    }
    for (auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->end_draw();
        }
    }
}

void App::terminate() {
    // We terminate controllers in reverse order of their registration to ensure that controllers that depend on other controllers are terminated last.
    for (auto it = m_controllers.rbegin(); it != m_controllers.rend(); ++it) {
        auto controller = *it;
        controller->terminate();
        spdlog::info("{}::terminate", controller->name());
    }
}

void App::app_setup() {
    RG_UNIMPLEMENTED("You should override App::app_setup in your App implementation.");
}

void App::handle_error(const util::Error &e) {
    spdlog::error(e.report());
}
} // namespace engine

