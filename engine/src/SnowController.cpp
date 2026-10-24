#include <glad/glad.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/SnowController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Utils.hpp>

void SnowController::initialize() {
    m_shader = get<engine::resources::ResourcesController>()->shader("snow");
    // Flakes have no vertex data, but core OpenGL still requires a vertex array to be bound for drawing.
    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
}

void SnowController::terminate() {
    CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
}

void SnowController::update() {
    const float dt     = get<engine::platform::PlatformController>()->dt();
    const float target = snow_enabled ? snow_intensity : 0.0f;
    m_intensity        = engine::util::approach(m_intensity, target, dt / FADE_SECONDS);
    const bool snowing = m_intensity > 0.0f;
    const float rate   = snowing ? m_intensity / COVERING_SECONDS : 1.0f / MELTING_SECONDS;
    m_cover            = engine::util::approach(m_cover, snowing ? 1.0f : 0.0f, dt * rate);
}

glm::vec3 SnowController::fall_velocity() const {
    const auto &wind = get<engine::graphics::Renderer>()->wind;
    // The same direction the vegetation sways in (see grass_shader.glsl).
    const glm::vec2 drift = wind.enabled ? glm::normalize(glm::vec2(1.0f, 0.5f)) * wind.intensity * WIND_DRIFT : glm::vec2(0.0f);
    return {drift.x, -fall_speed, drift.y};
}

void SnowController::draw() {
    const auto flake_count = static_cast<int32_t>(static_cast<float>(MAX_FLAKES) * m_intensity);
    if (flake_count == 0) {
        return;
    }
    get<engine::graphics::GraphicsController>()->profiler().begin_section("Snow");

    m_shader->use();
    m_shader->set_vec3("snowVolume", VOLUME_SIZE);
    m_shader->set_vec3("fallVelocity", fall_velocity());
    m_shader->set_float("flakeSize", flake_size);
    m_shader->set_float("opacity", opacity);

    // Flakes are depth tested against the scene but don't occlude each other, and don't feed the bloom buffer.
    engine::graphics::OpenGL::enable_blending();
    CHECKED_GL_CALL(glDepthMask, GL_FALSE);
    CHECKED_GL_CALL(glColorMaski, 1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glDrawArraysInstanced, GL_TRIANGLE_STRIP, 0, 4, flake_count);
    CHECKED_GL_CALL(glColorMaski, 1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    CHECKED_GL_CALL(glDepthMask, GL_TRUE);
    engine::graphics::OpenGL::disable_blending();
}
