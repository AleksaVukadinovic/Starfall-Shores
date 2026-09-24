/**
 * @file HeightField.hpp
 * @brief Ground heights sampled on a regular grid, for fast "how high is the ground here" queries.
 */

#pragma once

#include <optional>
#include <span>
#include <vector>
#include <glm/glm.hpp>

namespace engine::physics {

/**
 * @class HeightField
 * @brief The height of the highest surface above every point of a regular grid on the XZ plane.
 *
 * Built once from world-space triangles; queries interpolate between the four surrounding grid points, so they take
 * constant time regardless of how many triangles the ground has. Gaps up to @ref HeightField::MAX_GAP wide, like the
 * spaces between bridge planks, are filled in. Overhangs (e.g. caves) aren't represented.
 */
class HeightField {
public:
    HeightField() = default;

    /**
     * @param triangles World-space triangle corners, three consecutive points per triangle.
     * @param cell_size Distance between grid points; details smaller than this are smoothed out.
     */
    HeightField(std::span<const glm::vec3> triangles, float cell_size);

    /**
     * @returns The ground height at `xz`, or nothing if there is no ground there (outside the geometry, or in a hole).
     */
    [[nodiscard]] std::optional<float> height_at(glm::vec2 xz) const;

    /**
     * @brief Holes in the ground up to this wide, in world units, are filled by interpolating across them.
     */
    static constexpr float MAX_GAP = 1.0f;

private:
    void rasterize(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

    /**
     * @brief Fills runs of up to `max_cells` missing samples that have ground on both ends, along x and then along z.
     */
    void fill_gaps(int max_cells);

    [[nodiscard]] std::size_t index(const int x, const int z) const {
        return static_cast<std::size_t>(z) * m_size.x + x;
    }

    glm::vec2 m_origin{0.0f};
    float m_cell_size{1.0f};
    glm::ivec2 m_size{0};
    /**
     * @brief Height per grid point, row by row along x; NaN where no triangle covers the point.
     */
    std::vector<float> m_heights;
};

} // namespace engine::physics
