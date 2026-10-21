#include <glad/glad.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/ParticleController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>
#include <cmath>

namespace engine::graphics {

void ParticleController::initialize() {
    m_shader = get<resources::ResourcesController>()->shader("particles");
    // Particles have no vertex data, but core OpenGL still requires a vertex array to be bound for drawing.
    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
}

void ParticleController::terminate() {
    CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
}

void ParticleController::add_emitter(const ParticleEmitter &emitter) {
    RG_GUARANTEE(emitter.count <= MAX_PARTICLES_PER_EMITTER, "Particle emitter {} has {} particles, the maximum is {}.",
                 emitter.name, emitter.count, MAX_PARTICLES_PER_EMITTER);
    m_emitters.push_back({.emitter = emitter, .visible = true, .intensity = 0.0f});
}

void ParticleController::set_group_visible(const std::string_view group, const bool visible) {
    for (auto &state: m_emitters) {
        if (state.emitter.group == group) {
            state.visible = visible;
        }
    }
}

void ParticleController::update() {
    const float dt = get<platform::PlatformController>()->dt();
    for (auto &state: m_emitters) {
        const float fade = state.emitter.fade_seconds > 0.0f ? dt / state.emitter.fade_seconds : 1.0f;
        state.intensity  = util::approach(state.intensity, state.visible ? 1.0f : 0.0f, fade);
    }
}

void ParticleController::draw() {
    const auto &wind = get<Renderer>()->wind;
    // The same direction the vegetation sways in (see grass_shader.glsl).
    const glm::vec2 wind_direction = glm::normalize(glm::vec2(1.0f, 0.5f));
    const glm::vec3 wind_velocity  = wind.enabled ? glm::vec3(wind_direction.x, 0.0f, wind_direction.y) * wind.intensity * WIND_SPEED : glm::vec3(0.0f);

    bool profiled = false;
    for (const auto &state: m_emitters) {
        if (state.intensity <= 0.0f) {
            continue;
        }
        if (!profiled) {
            get<GraphicsController>()->profiler().begin_section("Particles");
            profiled = true;
        }
        draw_emitter(state, wind_velocity);
    }
    if (profiled) {
        CHECKED_GL_CALL(glDepthMask, GL_TRUE);
        OpenGL::disable_blending();
    }
}

void ParticleController::draw_emitter(const EmitterState &state, const glm::vec3 &wind_velocity) const {
    const ParticleEmitter &emitter = state.emitter;
    // Fading out draws fewer particles; the shader fades the last one in smoothly.
    const float visible_count = static_cast<float>(emitter.count) * state.intensity;

    m_shader->use();
    m_shader->set_vec3("emitterPosition", emitter.position);
    m_shader->set_vec3("emitterExtent", emitter.extent);
    m_shader->set_bool("followCamera", emitter.follow_camera);
    m_shader->set_float("visibleCount", visible_count);
    m_shader->set_float("lifetime", emitter.lifetime);
    m_shader->set_float("lifetimeVariation", emitter.lifetime_variation);
    m_shader->set_vec3("velocity", emitter.velocity + wind_velocity * emitter.wind_influence);
    m_shader->set_vec3("velocityVariation", emitter.velocity_variation);
    m_shader->set_vec3("acceleration", emitter.acceleration);
    m_shader->set_float("turbulence", emitter.turbulence);
    m_shader->set_float("turbulenceFrequency", emitter.turbulence_frequency);
    m_shader->set_vec2("size", glm::vec2(emitter.size_start, emitter.size_end));
    m_shader->set_vec4("colorStart", emitter.color_start);
    m_shader->set_vec4("colorEnd", emitter.color_end);
    m_shader->set_bool("additive", emitter.blend == ParticleBlend::Additive);
    m_shader->set_bool("lit", emitter.lit);
    m_shader->set_float("blinkFrequency", emitter.blink_frequency);

    // Particles are depth tested against the scene but don't hide each other.
    CHECKED_GL_CALL(glEnable, GL_BLEND);
    if (emitter.blend == ParticleBlend::Additive) {
        CHECKED_GL_CALL(glBlendFunc, GL_ONE, GL_ONE);
    } else {
        CHECKED_GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    CHECKED_GL_CALL(glDepthMask, GL_FALSE);
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glDrawArraysInstanced, GL_TRIANGLE_STRIP, 0, 4, static_cast<int32_t>(std::ceil(visible_count)));
}

}
