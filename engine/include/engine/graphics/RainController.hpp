/**
 * @file RainController.hpp
 * @brief Volumetric rain: falling rain streaks around the camera, plus the wetness it leaves on the scene.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>

namespace engine::resources {
class Shader;
}

/**
 * @class RainController
 * @brief Draws rain drops as lit, camera-facing streaks in a volume that follows the camera.
 *
 * Drops are generated entirely on the GPU: the vertex shader derives each drop's position from its instance id and
 * the time, so there is no per-drop CPU work and no buffer to update. Drops are fixed in world space and wrap around
 * the volume, so moving the camera doesn't drag the rain along.
 *
 * Rain fades in and out smoothly, and surfaces get wet while it rains and dry slowly afterwards; the current
 * intensity and wetness are sent to all scene shaders through the frame uniforms.
 *
 * Draw it after the opaque scene and the skybox, so drops are hidden behind geometry and blend over the sky.
 */
class RainController final : public engine::core::Controller {
public:
    bool rain_enabled = false;
    /**
     * @brief Target amount of rain in [0, 1]; the rain eases towards it.
     */
    float rain_intensity = 0.7f;
    /**
     * @brief Falling speed of the drops in world units per second.
     */
    float fall_speed  = 12.0f;
    float drop_length = 0.6f;
    float drop_width  = 0.012f;
    float opacity     = 0.35f;

    [[nodiscard]] std::string_view name() const override {
        return "RainController";
    }

    /**
     * @returns The current amount of rain in [0, 1], which follows @ref RainController::rain_intensity over a few seconds.
     */
    [[nodiscard]] float current_intensity() const {
        return m_intensity;
    }

    /**
     * @returns How wet the scene is in [0, 1]. Surfaces get wet while it rains and dry slowly afterwards.
     */
    [[nodiscard]] float wetness() const {
        return m_wetness;
    }

private:
    static constexpr uint32_t MAX_DROPS = 40000;
    /**
     * @brief Size of the box around the camera that the drops fill.
     */
    static constexpr glm::vec3 VOLUME_SIZE = glm::vec3(40.0f, 24.0f, 40.0f);
    static constexpr float FADE_SECONDS   = 3.0f;
    static constexpr float WETTING_SECONDS = 15.0f;
    static constexpr float DRYING_SECONDS = 45.0f;
    /**
     * @brief How much the wind slants the drops, per unit of wind intensity.
     */
    static constexpr float WIND_SLANT = 0.35f;

    void initialize() override;
    void update() override;
    void draw() override;
    void terminate() override;

    [[nodiscard]] glm::vec3 fall_velocity() const;

    const engine::resources::Shader *m_shader = nullptr;
    uint32_t m_vao    = 0;
    float m_intensity = 0.0f;
    float m_wetness   = 0.0f;
};
