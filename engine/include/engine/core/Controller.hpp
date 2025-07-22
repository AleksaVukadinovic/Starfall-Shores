/**
 * @file Controller.hpp
 * @brief Defines the Controller class that serves as the base class for all controllers in the engine.
 */

#ifndef MATF_RG_PROJECT_CONTROLLER_HPP
#define MATF_RG_PROJECT_CONTROLLER_HPP

#include <engine/util/Errors.hpp>
#include <memory>
#include <string_view>
#include <vector>
#include <typeinfo>

namespace engine::core {
/**
* @class Controller
* @brief Controllers are a hook into the @ref App `main loop` execution.
* By overriding virtual functions of this class the user can
* execute custom code during each of the `main loop` phases.
*
* Register controller with @ref App::register_controller during the @ref App::app_setup function.
*
* Every controller instance is a singleton instance that is managed by the @ref App.
* There can be no two instances of the same controller.
*
* \code
* class LoggingController : public engine::core::Controller {
* public:
*   void initialize() override { spdlog::log("Logging::initialize"); }
*   void terminate() override { spdlog::log("Logging::terminate"); }
* };
* \endcode
* To have the engine execute the code from the `LoggingController` we must first register it:
* \code
* void MyApp::app_setup() {
*   register_controller<LoggingController>();
* }
* \endcode
*/
class Controller {
    friend class App;

public:
    /**
    * @brief Serves as a single access point for all the Controller types throughout the code base.
    * Make sure to register the controller via @ref App::register_controller before calling this function.
    *
    * @returns The only instance of the TController.
    */
    template<typename TController>
    static TController *get(std::source_location location = std::source_location::current()) {
        static_assert(std::is_base_of_v<Controller, TController>);
        static TController *controller = create_if_absent<TController>();
        RG_GUARANTEE(controller->is_registered(),
                     "Trying to get an unregistered controller in: {}:{}.\nPlease call register_controller<> first during App::app_setup.",
                     location.file_name(), location.line());
        return controller;
    }

    /**
    * Returns the controller class name; used for logging.
    * @return Controller name
    */
    virtual std::string_view name() const {
        return typeid(*this).name();
    }

    virtual ~Controller() = default;

    /**
    * Orders the controller `next` to be after `this` controller.
    * All the virtual member methods of the controller `this` will always execute
    * before the member functions of the controller `next`.
    */
    void before(Controller *next) {
        this->m_next
            .push_back(next);
    }

    /**
    * Orders the controller `this` to be after `prev` controller.
    * All the virtual member methods of the controller `prev` will always execute
    * before the member functions of the controller `this`.
    */
    void after(Controller *prev) {
        prev->before(this);
    }

    /**
    * Returns the controllers that have a direct dependency to `this` controller.
    * @returns Controllers executing after `this`
    */
    const std::vector<Controller *> &next() const {
        return m_next;
    }

    /**
    * @brief Controller will execute as long this function returns true.
    *
    * You can turn the controller on/off by calling @ref Controller::set_enable.
    */
    bool is_enabled() const {
        return m_enabled;
    }

    /**
    * @brief Enables or disables the controller based on value.
    * The @ref engine::core::App executes only the enabled controllers,
    * except for the @ref Controller::poll_events function, that's executed always.
    * By default, controllers are enabled when registered.
    */
    void set_enable(bool value) {
        m_enabled = value;
    }

private:
    void mark_as_registered() {
        m_registered = true;
    }

    bool is_registered() const {
        return m_registered;
    }

    /**
    * @brief Initializes the controller. Executes in the @ref core::App::initialize.
    */
    virtual void initialize() {
    }

    /**
    * @brief Checks whether the main loop should continue. Executes in the @ref core::App::loop.
    * @returns true if the render loop should continue.
    */
    virtual bool loop() {
        return true;
    }

    /**
    * @brief Process internal and external events. Executes in the @ref core::App::poll_events.
    */
    virtual void poll_events() {
    }

    /**
    * @brief Update the controller state and prepare for drawing. Executes in the @ref core::App::update.
    */
    virtual void update() {
    }

    /**
    * @brief Perform preparation for drawing. Executes in the @ref core::App::draw, before @ref core::Controller::draw.
    */
    virtual void begin_draw() {
    }

    /**
    * @brief Draw the world state. Executes in the @ref core::App::draw.
    */
    virtual void draw() {
    }

    /**
    * @brief Finalize drawing. Executes in the @ref core::App::draw, after @ref engine::core::Controller::draw.
    */
    virtual void end_draw() {
    }

    /**
    * @brief Terminate the controller. Executes in the @ref core::App::terminate.
    *
    * Note that the `terminate` executes in the reverse order from initialize.
    */
    virtual void terminate() {
    }

    template<typename TController>
    static TController *create_if_absent() {
        static_assert(std::is_base_of_v<Controller, TController>);
        static std::unique_ptr<TController> controller = std::make_unique<TController>();
        return controller.get();
    }

    /**
     * @brief List of controllers that are dependent on this controller.
     * If controller `A` is in the `m_next`, that means that `this` Controller will execute before
     * the controller `A`.
     */
    std::vector<Controller *> m_next{};

    /**
     * @brief Internal Controller field used to ensure that the controller isn't registered twice.
     */
    bool m_registered{false};

    /**
    * @brief Internal field used to control weather the @ref engine::core::App executes the controller.
    */
    bool m_enabled{true};
};

/**
 * @class EngineControllersBegin
 * @brief This controller does nothing and together with @ref EngineControllersEnd it servers as a sentinel
 * controller for marking the engine controllers.
 *
 * This makes it easier for the concrete `App` implementation to order its custom controllers before/after
 * engine controllers.
 *
 * Here is an example of how to make the `user_controller` execute before all the engine controllers:
 * @code
 * auto user_controller = ...;
 * user_controller->before(engine::core::Controller::get<EngineControllersBegin>());
 * @endcode
 */
class EngineControllersBegin final : public Controller {
public:
    std::string_view name() const override {
        return "EngineControllersBegin";
    }
};

/**
 * @class EngineControllersEnd
 * @brief This controller does nothing and together with @ref EngineControllersEnd it servers as a sentinel
 * controller for marking the engine controllers.
 *
 * This makes it easier for the concrete `App` implementation to order its custom controllers before/after
 * engine controllers.
 *
 * Here is an example of how to make the `user_controller` execute after all the engine controllers:
 * @code
 * auto user_controller = ...;
 * user_controller->after(engine::core::Controller::get<EngineControllersEnd>());
 * @endcode
 */
class EngineControllersEnd final : public Controller {
public:
    std::string_view name() const override {
        return "EngineControllersEnd";
    }
};
} // namespace engine

#endif//MATF_RG_PROJECT_CONTROLLER_HPP
