/**
 * @file FPSCameraController.hpp
 * @brief Provides a ready-to-use first-person camera controller with WASD movement and mouse look.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/platform/Input.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <optional>

namespace engine::graphics {
class Camera;

/**
 * @class FPSCameraController
 * @brief Optional engine controller that provides standard first-person camera controls.
 *
 * Handles WASD/arrow-key movement, mouse look, vertical movement (Space/C),
 * sprint (Left Shift), and cursor toggle (P).
 *
 * In walk mode the camera walks on the ground given by @ref FPSCameraController::ground_height instead of flying:
 * it keeps @ref FPSCameraController::eye_height above the ground, falls with gravity, jumps with Space,
 * can't climb slopes steeper than @ref FPSCameraController::max_slope, and can't leave the walkable ground.
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

    bool walk_mode   = false;
    float eye_height = 1.7f;
    float walk_speed = 4.0f;
    float run_speed  = 9.0f;
    float jump_speed = 5.5f;
    float gravity    = 20.0f;
    /**
     * @brief Steepest slope that can be walked up, as height gained per unit of distance (1 is 45 degrees).
     */
    float max_slope  = 1.0f;

    /**
     * @brief Height of the walkable ground at a point on the XZ plane, or nothing where the walker can't go.
     * Walk mode has no effect until this is set.
     */
    std::function<std::optional<float>(glm::vec2)> ground_height;

private:
    /**
     * @brief How far below the feet the ground may drop while walking and still be followed, instead of falling.
     */
    static constexpr float GROUND_SNAP_DISTANCE = 0.3f;

    void initialize() override;
    void update() override;

    void fly(float dt) const;
    void walk(float dt);
    [[nodiscard]] glm::vec3 walk_direction() const;

    Camera *m_camera = nullptr;
    float m_vertical_speed = 0.0f;
    bool m_on_ground = false;
};
}
