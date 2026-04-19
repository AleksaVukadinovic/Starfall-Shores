/**
 * @file FPSCameraController.hpp
 * @brief Provides a ready-to-use first-person camera controller with WASD movement and mouse look.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/platform/Input.hpp>

namespace engine::graphics {
class Camera;

/**
 * @class FPSCameraController
 * @brief Optional engine controller that provides standard first-person camera controls.
 *
 * Handles WASD/arrow-key movement, mouse look, vertical movement (Space/C),
 * sprint (Left Shift), and cursor toggle (P).
 *
 * Register it in your App::app_setup to enable:
 * @code
 * auto fps_camera = register_controller<engine::graphics::FPSCameraController>();
 * fps_camera->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
 * @endcode
 *
 * All key bindings and speed values are public and can be reconfigured after registration.
 */
class FPSCameraController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "FPSCameraController";
    }

    float normal_speed = 7.0f;
    float sprint_speed = 20.0f;

    platform::KeyId key_forward      = platform::KEY_W;
    platform::KeyId key_forward_alt  = platform::KEY_UP;
    platform::KeyId key_backward     = platform::KEY_S;
    platform::KeyId key_backward_alt = platform::KEY_DOWN;
    platform::KeyId key_left         = platform::KEY_A;
    platform::KeyId key_left_alt     = platform::KEY_LEFT;
    platform::KeyId key_right        = platform::KEY_D;
    platform::KeyId key_right_alt    = platform::KEY_RIGHT;
    platform::KeyId key_up           = platform::KEY_SPACE;
    platform::KeyId key_down         = platform::KEY_C;
    platform::KeyId key_sprint       = platform::KEY_LEFT_SHIFT;
    platform::KeyId key_rotate_left  = platform::KEY_Q;
    platform::KeyId key_rotate_right = platform::KEY_E;
    platform::KeyId key_toggle_cursor = platform::KEY_P;

    float keyboard_rotate_speed = 10.0f;

private:
    void initialize() override;
    void update() override;

    Camera *m_camera = nullptr;
};
}
