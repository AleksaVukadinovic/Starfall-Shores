/**
 * @file InstanceBuffer.hpp
 * @brief Per-instance transforms for drawing many copies of a model with a single draw call per mesh.
 */

#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <glm/glm.hpp>

namespace engine::resources {
class Model;

/**
 * @class InstanceBuffer
 * @brief Owns a GPU buffer of instance transforms and one vertex array per mesh of a @ref Model
 * that reads the mesh geometry together with the instance transforms (vertex attributes 5-8).
 *
 * All OpenGL objects are created once; drawing only needs to bind the prepared vertex arrays.
 */
class InstanceBuffer {
public:
    explicit InstanceBuffer(const Model &model);

    /**
     * @brief Replaces the buffer contents with `transforms`.
     * The previous storage is orphaned, so the upload never waits for draws that still read the old data.
     */
    void upload(std::span<const glm::mat4> transforms) const;

    [[nodiscard]] uint32_t vao(std::size_t mesh_index) const {
        return m_vaos[mesh_index];
    }

    void destroy() const;

private:
    uint32_t m_buffer{0};
    std::vector<uint32_t> m_vaos;
};

} // namespace engine::resources
