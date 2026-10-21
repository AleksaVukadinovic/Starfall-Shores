/**
 * @file ParticleController.hpp
 * @brief GPU particle effects such as sparks, smoke, fireflies and pollen.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {

enum class ParticleBlend {
    /**
     * @brief Particles add light, e.g. sparks or fireflies. Bright particles feed the bloom.
     */
    Additive,
    /**
     * @brief Particles cover what's behind them, e.g. smoke or dust.
     */
    Alpha
};

/**
 * @brief Describes where particles appear and how they move and look over their lifetime.
 * Every particle lives for about `lifetime` seconds, then starts again at a new random spot of the spawn box.
 */
struct ParticleEmitter {
    std::string name;
    /**
     * @brief Optional group of scene objects the emitter is shown and hidden with, see @ref ParticleController::set_group_visible.
     */
    std::string group;

    /**
     * @brief Center of the spawn box; relative to the camera when `follow_camera` is set.
     */
    glm::vec3 position{0.0f};
    /**
     * @brief Half the size of the spawn box.
     */
    glm::vec3 extent{0.5f};
    /**
     * @brief The particles stay in a box of 2 * `extent` around the camera, wrapping around its sides, e.g. for dust in the air.
     */
    bool follow_camera = false;
    uint32_t count     = 100;

    float lifetime = 3.0f;
    /**
     * @brief In [0, 1]: how much shorter than `lifetime` a particle's life can be.
     */
    float lifetime_variation = 0.3f;
    glm::vec3 velocity{0.0f};
    /**
     * @brief Each particle's velocity differs from `velocity` by up to this much on each axis.
     */
    glm::vec3 velocity_variation{0.0f};
    /**
     * @brief Constant acceleration, e.g. gravity (negative y) or buoyancy (positive y).
     */
    glm::vec3 acceleration{0.0f};
    /**
     * @brief Distance particles swirl around their path, and how many times per second they change direction.
     */
    float turbulence           = 0.0f;
    float turbulence_frequency = 1.0f;
    /**
     * @brief How strongly the wind carries the particles, from 0 (not at all) to 1 (as fast as the wind).
     */
    float wind_influence = 0.0f;

    float size_start = 0.1f;
    float size_end   = 0.1f;
    /**
     * @brief Color and opacity at the start and the end of a particle's life. Additive particles can be brighter than 1.
     */
    glm::vec4 color_start{1.0f};
    glm::vec4 color_end{1.0f};
    ParticleBlend blend = ParticleBlend::Additive;
    /**
     * @brief Lit particles take the color of the light around them (sun, sky, point lights), e.g. smoke; unlit ones glow.
     */
    bool lit = false;
    /**
     * @brief Blinks per second, each particle at its own time, e.g. fireflies; 0 for a steady glow.
     */
    float blink_frequency = 0.0f;
    /**
     * @brief Seconds the emitter takes to fade in or out when shown or hidden.
     */
    float fade_seconds = 2.0f;
};

/**
 * @class ParticleController
 * @brief Draws particle emitters entirely on the GPU.
 *
 * Particles have no stored state: the vertex shader computes every particle's position, size and color from its index
 * and the time, so emitters cost no CPU work and no buffer updates, and each one is a single instanced draw call.
 * The trade-off is that particles follow predefined paths and don't collide with the scene.
 *
 * Draw it after the opaque scene, the skybox and the water, since particles are transparent.
 */
class ParticleController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "ParticleController";
    }

    static constexpr uint32_t MAX_PARTICLES_PER_EMITTER = 20000;

    void add_emitter(const ParticleEmitter &emitter);

    /**
     * @brief Fades the emitters of `group` in or out.
     */
    void set_group_visible(std::string_view group, bool visible);

private:
    struct EmitterState {
        ParticleEmitter emitter;
        bool visible    = true;
        float intensity = 0.0f;
    };

    /**
     * @brief Speed of the wind at full wind intensity, in world units per second.
     */
    static constexpr float WIND_SPEED = 2.0f;

    void initialize() override;
    void update() override;
    void draw() override;
    void terminate() override;

    void draw_emitter(const EmitterState &state, const glm::vec3 &wind_velocity) const;

    const resources::Shader *m_shader = nullptr;
    uint32_t m_vao = 0;
    std::vector<EmitterState> m_emitters;
};

}
