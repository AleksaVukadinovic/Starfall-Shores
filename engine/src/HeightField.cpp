#include <engine/physics/HeightField.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace engine::physics {

static constexpr float NO_GROUND = std::numeric_limits<float>::quiet_NaN();

HeightField::HeightField(const std::span<const glm::vec3> triangles, const float cell_size)
    : m_cell_size(cell_size) {
    if (triangles.empty()) {
        return;
    }
    glm::vec2 min(std::numeric_limits<float>::max());
    glm::vec2 max(std::numeric_limits<float>::lowest());
    for (const auto &point: triangles) {
        min = glm::min(min, glm::vec2(point.x, point.z));
        max = glm::max(max, glm::vec2(point.x, point.z));
    }
    m_origin = min;
    m_size   = glm::ivec2(glm::ceil((max - min) / cell_size)) + 1;
    m_heights.assign(static_cast<std::size_t>(m_size.x) * m_size.y, NO_GROUND);
    for (std::size_t i = 0; i + 2 < triangles.size(); i += 3) {
        rasterize(triangles[i], triangles[i + 1], triangles[i + 2]);
    }
    fill_gaps(static_cast<int>(MAX_GAP / cell_size));
}

void HeightField::fill_gaps(const int max_cells) {
    // Walks one line of samples: `count` samples, `stride` apart in m_heights, starting at `first`.
    auto fill_line = [&](const std::size_t first, const std::size_t stride, const int count) {
        int last_ground = -1;
        for (int i = 0; i < count; ++i) {
            if (std::isnan(m_heights[first + i * stride])) {
                continue;
            }
            const int gap = i - last_ground - 1;
            if (last_ground >= 0 && gap > 0 && gap <= max_cells) {
                const float from = m_heights[first + last_ground * stride];
                const float to   = m_heights[first + i * stride];
                for (int j = 1; j <= gap; ++j) {
                    m_heights[first + (last_ground + j) * stride] = glm::mix(from, to, static_cast<float>(j) / static_cast<float>(gap + 1));
                }
            }
            last_ground = i;
        }
    };
    for (int z = 0; z < m_size.y; ++z) {
        fill_line(index(0, z), 1, m_size.x);
    }
    for (int x = 0; x < m_size.x; ++x) {
        fill_line(index(x, 0), m_size.x, m_size.y);
    }
}

void HeightField::rasterize(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    // Barycentric coordinates of each grid point inside the triangle's footprint on the XZ plane.
    const glm::vec2 a2(a.x, a.z), b2(b.x, b.z), c2(c.x, c.z);
    const float area = (b2.x - a2.x) * (c2.y - a2.y) - (c2.x - a2.x) * (b2.y - a2.y);
    if (std::abs(area) < 1e-8f) {
        return; // vertical or degenerate: it doesn't cover any ground
    }
    const glm::ivec2 first = glm::max(glm::ivec2(glm::ceil((glm::min(a2, glm::min(b2, c2)) - m_origin) / m_cell_size)), glm::ivec2(0));
    const glm::ivec2 last  = glm::min(glm::ivec2(glm::floor((glm::max(a2, glm::max(b2, c2)) - m_origin) / m_cell_size)), m_size - 1);
    static constexpr float EDGE_TOLERANCE = -1e-4f;
    for (int z = first.y; z <= last.y; ++z) {
        for (int x = first.x; x <= last.x; ++x) {
            const glm::vec2 p  = m_origin + glm::vec2(x, z) * m_cell_size;
            const float weight_b = ((p.x - a2.x) * (c2.y - a2.y) - (c2.x - a2.x) * (p.y - a2.y)) / area;
            const float weight_c = ((b2.x - a2.x) * (p.y - a2.y) - (p.x - a2.x) * (b2.y - a2.y)) / area;
            const float weight_a = 1.0f - weight_b - weight_c;
            if (weight_a < EDGE_TOLERANCE || weight_b < EDGE_TOLERANCE || weight_c < EDGE_TOLERANCE) {
                continue;
            }
            const float surface = weight_a * a.y + weight_b * b.y + weight_c * c.y;
            float &ground       = m_heights[index(x, z)];
            ground              = std::isnan(ground) ? surface : std::max(ground, surface);
        }
    }
}

std::optional<float> HeightField::height_at(const glm::vec2 xz) const {
    const glm::vec2 grid  = (xz - m_origin) / m_cell_size;
    const glm::ivec2 cell = glm::ivec2(glm::floor(grid));
    if (cell.x < 0 || cell.y < 0 || cell.x + 1 >= m_size.x || cell.y + 1 >= m_size.y) {
        return std::nullopt;
    }
    const float h00 = m_heights[index(cell.x, cell.y)], h10 = m_heights[index(cell.x + 1, cell.y)];
    const float h01 = m_heights[index(cell.x, cell.y + 1)], h11 = m_heights[index(cell.x + 1, cell.y + 1)];
    if (std::isnan(h00) || std::isnan(h10) || std::isnan(h01) || std::isnan(h11)) {
        return std::nullopt;
    }
    const glm::vec2 t = grid - glm::vec2(cell);
    return glm::mix(glm::mix(h00, h10, t.x), glm::mix(h01, h11, t.x), t.y);
}

} // namespace engine::physics
