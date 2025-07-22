/**
 * @file App.hpp
 * @brief Defines the base App class that serves as the application core structure and the entry point.
 */

#ifndef APP_H
#define APP_H

namespace engine::util {
class Error;
}

#include <vector>

namespace engine::core {
class Controller;

/**
* @class App
* @brief Defines the base App class that serves as the application core structure and the entry point.
*
* The main function calls @ref App::run that defines the core structure of the application.
* You can hook into the engine execution by overriding the desired methods in your
* implementations of the @ref engine::core::Controller class and registering your implementations
* via @ref App::register_controller.
* @code
* int App::run(int argc, char **argv) {
*    try {
*        engine_setup(argc, argv);
*        app_setup();
*        initialize();
*        while (loop()) {
*            poll_events();
*            update();
*            draw();
*        }
*        terminate();
*    } catch (const Error &e) {
*        handle_error(e);
*        terminate();
*    }
*    return on_exit();
* }
* @endcode
*/
class App {
public:
    /**
    * @brief The main entry point into the App.
    * @code
    * int App::run(int argc, char **argv) {
    *    try {
    *        engine_setup(argc, argv);
    *        app_setup();
    *        initialize();
    *        while (loop()) {
    *            poll_events();
    *            update();
    *            draw();
    *        }
    *        terminate();
    *    } catch (const Error &e) {
    *        handle_error(e);
    *        terminate();
    *    }
    *    return on_exit();
    * }
    * @endcode
    */
    int run(int argc, char **argv);

private:
    /**
    * @brief The first function that the engine calls to do its internal Controller classes `engine_setup`.
    */
    void engine_setup(int argc, char **argv);

    /**
    * @brief Override to define your custom app setup that gets called after the `engine_setup`.
    */
    virtual void app_setup();

    /**
    * @brief Initializes all the controllers registered in @ref App::app_setup. Calls @ref engine::core::Controller::initialize for registered controllers.
    *
    * After this functions finishes all the controllers have been initialized, and they can be now used
    * by calling @ref engine::core::Controller::get<TController>()
    */
    void initialize();

    /**
    * @brief Processes all pending events. Calls @ref engine::core::Controller::poll_events for registered controllers.
    *
    * It handles input events, system events, and any other types of events
    * that have been queued. This is where the platform events are processed.
    */
    void poll_events();

    /**
    * @brief Checks whether the render loop should continue executing. Calls @ref engine::core::Controller::loop for registered controllers.
    *
    * This is where you should check if an internal or external event happened
    * that stops the main render loop. For example the user presses ESC.
    *
    * @returns true if the main loop should continue, false otherwise.
    */
    bool loop();

    /**
    * @brief Updates the app logic state. Calls @ref engine::core::Controller::update for registered controllers.
    *
    * This is where all the App state should be updated including handling events
    * registered in @ref App::poll_events, processing physics, world logic etc.
    */
    void update();

    /**
    * @brief Draws the frame. Calls @ref engine::core::Controller::draw for registered controllers.
    *
    * This is where all the drawing should happen based on the state
    * that the @ref App::update computed.
    */
    void draw();

    /**
    * @brief Terminates the app. Calls @ref engine::core::Controller::terminate for registered controllers in the **reverse order**.
    *
    * Terminate is called always, regardless of whether the app closes successfully or an error occurs.
    */
    void terminate();

    /**
    * @brief Called right before the App exits.
    *
    * This where you can do custom operation when all the app state has already been terminated,
    * and all the internal engine systems have been shutdown.
    * This functions gets called just before the return from main.
    *
    * @returns The value that should be returned from the int main(...).
    */
    virtual int on_exit() {
        return 0;
    }

    virtual void handle_error(const util::Error &);

protected:
    /**
    * @brief Registers the controller for execution.
    * The Controller instance that the register_controller returns isn't initialized yet.
    * It will be initialized during the @ref App::initialize.
    * If the register_controller is called twice for the same controller, it's registered only once.
    * The other calls just return the pointer to the already registered instance.
    * @returns Pointer to the only instance of the provided Controller class TController.
    */
    template<typename TController>
    TController *register_controller() {
        TController *controller = TController::template create_if_absent<TController>();
        if (!controller->is_registered()) {
            m_controllers.push_back(controller);
            controller->mark_as_registered();
        }
        return controller;
    }

public:
    virtual ~App() = default;

private:
    std::vector<Controller *> m_controllers;
};
} // namespace engine

#endif//APP_H

