#include <spdlog/spdlog.h>
#include <engine/core/App.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Errors.hpp>

#include <engine/util/ArgParser.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/util/Utils.hpp>
#include <ranges>

namespace engine::core {
int App::run(const int argc, char **argv) {
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

void App::engine_setup(const int argc, char **argv) {
    util::ArgParser::instance()->initialize(argc, argv);
    util::Configuration::instance()->initialize();

    // register engine controllers
    const auto begin = register_controller<EngineControllersBegin>();
    const auto platform = register_controller<platform::PlatformController>();
    const auto graphics = register_controller<graphics::GraphicsController>();
    const auto resources = register_controller<resources::ResourcesController>();
    const auto end = register_controller<EngineControllersEnd>();
    begin->before(platform);
    platform->before(graphics);
    graphics->before(resources);
    resources->before(end);
}

void App::initialize() {
    // Topologically sort controllers based on their dependency graph formed by before/after methods.
    {
        auto adjacent_controllers = [](const Controller *curr) {
            return curr->next();
        };
        RG_GUARANTEE(!util::alg::has_cycle(range(m_controllers), adjacent_controllers),
                     "Please make sure that there are no cycles in the controller dependency graph.");
        util::alg::topological_sort(range(m_controllers), adjacent_controllers);
    }
    for (const auto controller: m_controllers) {
        spdlog::info(std::format("{}::initialize", controller->name()));
        controller->initialize();
    }
}

bool App::loop() const {
    for (const auto controller: m_controllers) {
        if (controller->is_enabled() && !controller->loop()) {
            return false;
        }
    }
    return true;
}

void App::poll_events() const {
    for (const auto controller: m_controllers) {
        // We don't check if the controller is enabled for poll_events because the controller may enable itself in the poll_events if it needs to.
        // For example, a GUIController may enable itself in the poll_events method if a button to enable/disable the GUI was pressed.
        controller->poll_events();
    }
}

void App::update() const {
    for (const auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->update();
        }
    }
}

void App::draw() const {
    for (const auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->begin_draw();
        }
    }
    for (const auto controller: m_controllers) {
        if (controller->is_enabled()) {
            controller->draw();
        }
    }
    for (const auto controller : std::views::reverse(m_controllers)) {
        if (controller->is_enabled()) {
            controller->end_draw();
        }
    }
}

void App::terminate() {
    // We terminate controllers in reverse order of their registration to ensure that controllers that depend on other controllers are terminated last.
    for (const auto controller : std::views::reverse(m_controllers)) {
        controller->terminate();
        spdlog::info(std::format("{}::terminate", controller->name()));
    }
}

void App::app_setup() {
    RG_UNIMPLEMENTED("You should override App::app_setup in your App implementation.");
}

void App::handle_error(const util::Error &e) {
    spdlog::error(e.report());
}
} // namespace engine

