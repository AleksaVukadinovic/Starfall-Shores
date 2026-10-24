/**
 * @file SnowController.hpp
 * @brief Snowfall: drifting snowflakes around the camera, plus the snow cover they leave on the scene.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>

namespace engine::resources {
class Shader;
}

/**
 * @class SnowController
 * @brief Draws snowflakes as soft, camera-facing discs in a volume that follows the camera.
 *
 * Like the rain, flakes are generated entirely on the GPU from their instance id and the time, are fixed in world space
 * and wrap around the volume. They fall slowly, flutter from side to side and drift with the wind.
 *
 * Snowfall fades in and out smoothly. While it snows, upward-facing surfaces above the water get covered, patchily at
 * first and faster the heavier it snows; the cover melts slowly afterwards. The current cover is sent to all scene
 * shaders through the frame uniforms.
 *
 * Draw it after the opaque scene and the skybox, so flakes are hidden behind geometry and blend over the sky.
 */
class SnowController final : public engine::core::Controller {
public:
    bool snow_enabled = false;
    /**
     * @brief Target amount of snow in [0, 1]; the snowfall eases towards it.
     */
    float snow_intensity = 0.7f;
    /**
     * @brief Falling speed of the flakes in world units per second.
     */
    float fall_speed = 1.2f;
    /**
     * @brief Radius of a flake in world units.
     */
    float flake_size = 0.025f;
    float opacity    = 0.85f;

    [[nodiscard]] std::string_view name() const override {
        return "SnowController";
    }

    /**
     * @returns The current amount of snowfall in [0, 1], which follows @ref SnowController::snow_intensity over a few seconds.
     */
    [[nodiscard]] float current_intensity() const {
        return m_intensity;
    }

    /**
     * @returns How much of the scene is covered in snow, in [0, 1]. It builds up while it snows and melts slowly afterwards.
     */
    [[nodiscard]] float cover() const {
        return m_cover;
    }

private:
    static constexpr uint32_t MAX_FLAKES = 30000;
    /**
     * @brief Size of the box around the camera that the flakes fill.
     */
    static constexpr glm::vec3 VOLUME_SIZE = glm::vec3(40.0f, 20.0f, 40.0f);
    static constexpr float FADE_SECONDS    = 3.0f;
    /**
     * @brief How long full snowfall takes to cover the scene; lighter snow takes longer.
     */
    static constexpr float COVERING_SECONDS = 60.0f;
    static constexpr float MELTING_SECONDS  = 90.0f;
    /**
     * @brief Horizontal drift of the flakes in world units per second, per unit of wind intensity.
     */
    static constexpr float WIND_DRIFT = 1.5f;

    void initialize() override;
    void update() override;
    void draw() override;
    void terminate() override;

    [[nodiscard]] glm::vec3 fall_velocity() const;

    const engine::resources::Shader *m_shader = nullptr;
    uint32_t m_vao    = 0;
    float m_intensity = 0.0f;
    float m_cover     = 0.0f;
};
