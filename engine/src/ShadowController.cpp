#include <engine/graphics/ShadowController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {

void ShadowController::initialize() {
    m_shadow_map = OpenGL::create_shadow_map(MAP_SIZE, CASCADE_COUNT);
}

void ShadowController::terminate() {
    if (m_shadow_map.fbo) {
        OpenGL::destroy_shadow_map(m_shadow_map);
        m_shadow_map = {};
    }
}

void ShadowController::begin_draw() {
    if (!enabled) {
        return;
    }
    update_light_view();
    fit_cascades(get<GraphicsController>()->camera()->Position);
}

void ShadowController::invalidate() {
    for (auto &cascade: m_cascades) {
        cascade.stale = true;
    }
}

void ShadowController::update_light_view() {
    const glm::vec3 direction = glm::normalize(light_target - light_position);
    if (glm::dot(direction, m_light_direction) > LIGHT_DIRECTION_TOLERANCE) {
        return;
    }
    m_light_direction  = direction;
    const glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0.0f, 0.0f, -1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    m_light_view       = glm::lookAt(glm::vec3(0.0f), direction, up);
    invalidate();
}

void ShadowController::fit_cascades(const glm::vec3 &camera_position) {
    const glm::vec3 camera = glm::vec3(m_light_view * glm::vec4(camera_position, 1.0f));
    for (std::size_t i = 0; i < CASCADE_COUNT; ++i) {
        Cascade &cascade     = m_cascades[i];
        const float radius   = CASCADE_RADII[i];
        const glm::vec3 drift = glm::abs(camera - cascade.center);
        const bool camera_left_cascade = std::max(drift.x, drift.y) > radius * RECENTER_FRACTION ||
                                         drift.z > DEPTH_TOWARDS_LIGHT * RECENTER_FRACTION;
        if (!cascade.stale && !camera_left_cascade) {
            continue;
        }
        // Snap the center to whole texels, so re-centering doesn't make shadow edges crawl.
        const float texel    = texel_size(i);
        cascade.center       = glm::vec3(glm::floor(glm::vec2(camera) / texel) * texel, camera.z);
        // The light looks down -Z in its view space, so a point at z has depth -z.
        const float depth    = -cascade.center.z;
        const auto projection = glm::ortho(cascade.center.x - radius, cascade.center.x + radius,
                                           cascade.center.y - radius, cascade.center.y + radius,
                                           depth - DEPTH_TOWARDS_LIGHT, depth + DEPTH_AWAY_FROM_LIGHT);
        cascade.light_space = projection * m_light_view;
        cascade.stale       = true;
    }
}

void ShadowController::begin_depth_pass(const std::size_t cascade) {
    OpenGL::begin_shadow_pass(m_shadow_map, MAP_SIZE, cascade);
    m_cascades[cascade].stale = false;
}

void ShadowController::end_depth_pass() const {
    const auto platform = get<platform::PlatformController>();
    OpenGL::end_shadow_pass(platform->window()->width(), platform->window()->height());
}

void ShadowController::bind_shadow_map() const {
    OpenGL::bind_texture_array_to_unit(m_shadow_map.texture, SHADOW_MAP_TEXTURE_UNIT);
}

}
