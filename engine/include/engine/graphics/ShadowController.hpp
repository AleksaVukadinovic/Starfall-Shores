/**
 * @file ShadowController.hpp
 * @brief Cascaded shadow maps for the main directional light, re-rendered only when they go stale.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/ShaderInterface.hpp>
#include <array>
#include <glm/glm.hpp>

namespace engine::graphics {

/**
 * @class ShadowController
 * @brief Owns the cascaded shadow map of the directional light shining from `light_position` towards `light_target`.
 *
 * Each cascade is a square area around the camera; closer cascades cover smaller areas with sharper texels.
 * A cascade is re-rendered only when it goes stale: when the light direction changes, when the camera
 * moves far enough from the cascade center, or when @ref ShadowController::invalidate is called.
 * The @ref Renderer renders the stale cascades at the beginning of each frame.
 */
class ShadowController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "ShadowController";
    }

    static constexpr uint32_t MAP_SIZE         = 2048;
    static constexpr std::size_t CASCADE_COUNT = MAX_SHADOW_CASCADES;
    /**
     * @brief Half the side length of the area covered by each cascade, from the closest to the farthest.
     */
    static constexpr std::array<float, CASCADE_COUNT> CASCADE_RADII = {8.0f, 24.0f, 64.0f, 160.0f};

    bool enabled = true;

    glm::vec3 light_position = glm::vec3(0.0f, 60.0f, 0.0f);
    glm::vec3 light_target   = glm::vec3(0.0f, 15.0f, 0.0f);

    /**
     * @brief Marks all cascades as stale. Call it whenever a shadow caster moves.
     */
    void invalidate();

    [[nodiscard]] bool is_stale(const std::size_t cascade) const {
        return m_cascades[cascade].stale;
    }

    [[nodiscard]] const glm::mat4 &light_space_matrix(const std::size_t cascade) const {
        return m_cascades[cascade].light_space;
    }

    /**
     * @returns Size of a single shadow map texel of the cascade in world units.
     */
    [[nodiscard]] static constexpr float texel_size(const std::size_t cascade) {
        return 2.0f * CASCADE_RADII[cascade] / static_cast<float>(MAP_SIZE);
    }

    /**
     * @brief Starts rendering depth into `cascade` and marks the cascade as up to date.
     */
    void begin_depth_pass(std::size_t cascade);

    void end_depth_pass() const;

    void bind_shadow_map() const;

private:
    struct Cascade {
        /**
         * @brief Center of the cascade in light view space.
         */
        glm::vec3 center{0.0f};
        glm::mat4 light_space{1.0f};
        bool stale{true};
    };

    /**
     * @brief How far (as a fraction of its radius) the camera can move from a cascade center before the cascade is re-centered.
     */
    static constexpr float RECENTER_FRACTION = 0.25f;
    /**
     * @brief Depth range of every cascade around the camera, along the light direction. Covers tall casters above the camera.
     */
    static constexpr float DEPTH_TOWARDS_LIGHT   = 250.0f;
    static constexpr float DEPTH_AWAY_FROM_LIGHT = 250.0f;
    /**
     * @brief The light direction is considered changed when it rotates by more than ~0.25 degrees.
     */
    static constexpr float LIGHT_DIRECTION_TOLERANCE = 0.99999f;

    void initialize() override;

    void terminate() override;

    void begin_draw() override;

    void update_light_view();

    void fit_cascades(const glm::vec3 &camera_position);

    std::array<Cascade, CASCADE_COUNT> m_cascades{};
    glm::vec3 m_light_direction{0.0f};
    glm::mat4 m_light_view{1.0f};
    OpenGL::ShadowMap m_shadow_map{};
};

}
