#include <glad/glad.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Utils.hpp>

void RainController::initialize() {
    m_shader = get<engine::resources::ResourcesController>()->shader("rain");
    // Drops have no vertex data, but core OpenGL still requires a vertex array to be bound for drawing.
    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
}

void RainController::terminate() {
    CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
}

void RainController::update() {
    const float dt     = get<engine::platform::PlatformController>()->dt();
    const float target = rain_enabled ? rain_intensity : 0.0f;
    m_intensity        = engine::util::approach(m_intensity, target, dt / FADE_SECONDS);
    const float wetting_seconds = m_intensity > m_wetness ? WETTING_SECONDS : DRYING_SECONDS;
    m_wetness          = engine::util::approach(m_wetness, m_intensity, dt / wetting_seconds);
}

glm::vec3 RainController::fall_velocity() const {
    const auto &wind = get<engine::graphics::Renderer>()->wind;
    // The same direction the vegetation sways in (see grass_shader.glsl).
    const glm::vec2 slant = wind.enabled ? glm::normalize(glm::vec2(1.0f, 0.5f)) * wind.intensity * WIND_SLANT : glm::vec2(0.0f);
    return glm::normalize(glm::vec3(slant.x, -1.0f, slant.y)) * fall_speed;
}

void RainController::draw() {
    const auto drop_count = static_cast<int32_t>(static_cast<float>(MAX_DROPS) * m_intensity);
    if (drop_count == 0) {
        return;
    }
    get<engine::graphics::GraphicsController>()->profiler().begin_section("Rain");

    m_shader->use();
    m_shader->set_vec3("rainVolume", VOLUME_SIZE);
    m_shader->set_vec3("fallVelocity", fall_velocity());
    m_shader->set_float("dropLength", drop_length);
    m_shader->set_float("dropWidth", drop_width);
    m_shader->set_float("opacity", opacity);

    // Drops are depth tested against the scene but don't occlude each other, and don't feed the bloom buffer.
    engine::graphics::OpenGL::enable_blending();
    CHECKED_GL_CALL(glDepthMask, GL_FALSE);
    CHECKED_GL_CALL(glColorMaski, 1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glDrawArraysInstanced, GL_TRIANGLE_STRIP, 0, 4, drop_count);
    CHECKED_GL_CALL(glColorMaski, 1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    CHECKED_GL_CALL(glDepthMask, GL_TRUE);
    engine::graphics::OpenGL::disable_blending();
}
