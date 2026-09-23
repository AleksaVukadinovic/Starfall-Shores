#include <engine/util/Geometry.hpp>

namespace engine::util {

AABB AABB::padded(const float fraction) const {
    const glm::vec3 padding = (max - min) * fraction;
    return {min - padding, max + padding};
}

AABB AABB::transformed(const glm::mat4 &transform) const {
    // Arvo's method: transform the center, and project the extents onto the absolute value of the linear part.
    const glm::vec3 new_center = glm::vec3(transform * glm::vec4(center(), 1.0f));
    const glm::mat3 abs_linear = glm::mat3(glm::abs(glm::vec3(transform[0])),
                                           glm::abs(glm::vec3(transform[1])),
                                           glm::abs(glm::vec3(transform[2])));
    const glm::vec3 new_extents = abs_linear * extents();
    return {new_center - new_extents, new_center + new_extents};
}

Frustum::Frustum(const glm::mat4 &view_projection) {
    const glm::mat4 m = glm::transpose(view_projection);
    m_planes = {
            m[3] + m[0], // left
            m[3] - m[0], // right
            m[3] + m[1], // bottom
            m[3] - m[1], // top
            m[3] + m[2], // near
            m[3] - m[2], // far
    };
    for (auto &plane: m_planes) {
        plane /= glm::length(glm::vec3(plane));
    }
}

bool Frustum::intersects(const AABB &box) const {
    const glm::vec3 center  = box.center();
    const glm::vec3 extents = box.extents();
    for (const auto &plane: m_planes) {
        const glm::vec3 normal = glm::vec3(plane);
        const float radius     = glm::dot(glm::abs(normal), extents);
        if (glm::dot(normal, center) + plane.w < -radius) {
            return false;
        }
    }
    return true;
}

} // namespace engine::util
