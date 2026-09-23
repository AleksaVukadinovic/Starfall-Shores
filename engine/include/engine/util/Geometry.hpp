/**
 * @file Geometry.hpp
 * @brief Bounding volumes and view-frustum tests used for culling.
 */

#pragma once

#include <array>
#include <limits>
#include <glm/glm.hpp>

namespace engine::util {

/**
 * @struct AABB
 * @brief Axis-aligned bounding box. A default constructed box is empty and grows with @ref AABB::expand.
 */
struct AABB {
    glm::vec3 min{std::numeric_limits<float>::max()};
    glm::vec3 max{std::numeric_limits<float>::lowest()};

    void expand(const glm::vec3 &point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    [[nodiscard]] glm::vec3 center() const {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] glm::vec3 extents() const {
        return (max - min) * 0.5f;
    }

    /**
     * @brief Returns the box grown by `fraction` of its size on every side.
     */
    [[nodiscard]] AABB padded(float fraction) const;

    /**
     * @brief Returns the smallest AABB enclosing this box after applying `transform` to it.
     */
    [[nodiscard]] AABB transformed(const glm::mat4 &transform) const;
};

/**
 * @class Frustum
 * @brief The six planes of a view volume, extracted from a view-projection matrix.
 */
class Frustum {
public:
    Frustum() = default;

    /**
     * @brief Extracts the planes of `view_projection` (Gribb & Hartmann). Works for perspective and orthographic projections.
     */
    explicit Frustum(const glm::mat4 &view_projection);

    /**
     * @returns false only if `box` is guaranteed to be completely outside the frustum.
     */
    [[nodiscard]] bool intersects(const AABB &box) const;

private:
    std::array<glm::vec4, 6> m_planes{};
};

} // namespace engine::util
