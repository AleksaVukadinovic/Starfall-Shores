/**
 * @file WaterController.hpp
 * @brief An endless, animated ocean surface.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <array>
#include <glm/glm.hpp>

namespace engine::resources {
class Shader;
class Skybox;
}

namespace engine::graphics {

/**
 * @class WaterController
 * @brief Draws the ocean: a grid that follows the camera, displaced by a sum of Gerstner waves.
 *
 * The grid is dense near the camera and coarser towards the horizon, and it moves with the camera in steps of its
 * finest cell size, so it looks endless. Waves are evaluated from world positions, so they don't move with the grid.
 * The surface reflects `reflected_skybox`, shows sun glints and foam on wave crests, and gets rain ripples while it rains.
 *
 * Water is transparent: draw it after the opaque scene and the skybox.
 */
class WaterController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "WaterController";
    }

    static constexpr std::size_t WAVE_COUNT = 6;

    /**
     * @brief Height of the calm water surface.
     */
    float level = 9.2f;
    /**
     * @brief Amplitude of the largest wave, in world units.
     */
    float wave_height = 0.35f;
    /**
     * @brief Length of the largest wave, in world units. The other waves are progressively shorter.
     */
    float wavelength = 14.0f;
    /**
     * @brief In [0, 1]: 0 gives round sine waves, 1 gives sharp crests and wide troughs.
     */
    float choppiness = 0.6f;
    /**
     * @brief Direction the waves travel in, in degrees around the vertical axis.
     */
    float wind_direction = 35.0f;
    float wave_speed     = 1.0f;
    float foam           = 0.5f;
    /**
     * @brief Opacity of the water when looking straight down; it's always opaque at grazing angles.
     */
    float opacity           = 0.75f;
    glm::vec3 deep_color    = glm::vec3(0.01f, 0.07f, 0.11f);
    glm::vec3 shallow_color = glm::vec3(0.04f, 0.28f, 0.30f);

    /**
     * @brief The sky that the water reflects. Must be set before the water is drawn.
     */
    const resources::Skybox *reflected_skybox = nullptr;

private:
    /**
     * @brief Number of grid cells along each side.
     */
    static constexpr uint32_t GRID_RESOLUTION = 256;
    /**
     * @brief Distance from the camera to the edge of the grid, beyond the far plane.
     */
    static constexpr float GRID_EXTENT = 400.0f;
    /**
     * @brief Blend between linear (1) and cubic (0) spacing of grid lines; small values concentrate vertices near the camera.
     */
    static constexpr float GRID_LINEAR_SHARE = 0.1f;
    static constexpr uint32_t ENVIRONMENT_TEXTURE_UNIT = 4;

    /**
     * @brief Per wave: direction, wave number and amplitude packed as `(direction.x, direction.y, 2π / length, amplitude)`.
     */
    using Waves = std::array<glm::vec4, WAVE_COUNT>;

    void initialize() override;
    void draw() override;
    void terminate() override;

    void create_grid();
    [[nodiscard]] Waves waves() const;
    [[nodiscard]] static float grid_cell_size();

    const resources::Shader *m_shader = nullptr;
    uint32_t m_vao         = 0;
    uint32_t m_vbo         = 0;
    uint32_t m_ebo         = 0;
    uint32_t m_index_count = 0;
};

}
