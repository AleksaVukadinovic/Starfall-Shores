#include <glad/glad.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/InstanceBuffer.hpp>
#include <engine/resources/Model.hpp>

namespace engine::resources {

InstanceBuffer::InstanceBuffer(const Model &model) {
    static constexpr uint32_t FIRST_INSTANCE_ATTRIBUTE = 5;
    static constexpr std::size_t COLUMN_SIZE           = sizeof(glm::vec4);

    CHECKED_GL_CALL(glGenBuffers, 1, &m_buffer);
    m_vaos.resize(model.meshes().size());
    CHECKED_GL_CALL(glGenVertexArrays, static_cast<int32_t>(m_vaos.size()), m_vaos.data());

    for (std::size_t i = 0; i < m_vaos.size(); ++i) {
        CHECKED_GL_CALL(glBindVertexArray, m_vaos[i]);
        model.meshes()[i].attach_vertex_buffers();
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_buffer);
        // A mat4 attribute occupies four consecutive vec4 attribute slots.
        for (uint32_t column = 0; column < 4; ++column) {
            const uint32_t attribute = FIRST_INSTANCE_ATTRIBUTE + column;
            CHECKED_GL_CALL(glEnableVertexAttribArray, attribute);
            CHECKED_GL_CALL(glVertexAttribPointer, attribute, 4, GL_FLOAT, GL_FALSE, static_cast<int32_t>(sizeof(glm::mat4)),
                            reinterpret_cast<void *>(column * COLUMN_SIZE)); // NOLINT
            CHECKED_GL_CALL(glVertexAttribDivisor, attribute, 1);
        }
    }
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void InstanceBuffer::upload(const std::span<const glm::mat4> transforms) const {
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_buffer);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, static_cast<int64_t>(transforms.size_bytes()), transforms.data(), GL_STREAM_DRAW);
}

void InstanceBuffer::destroy() const {
    CHECKED_GL_CALL(glDeleteVertexArrays, static_cast<int32_t>(m_vaos.size()), m_vaos.data());
    CHECKED_GL_CALL(glDeleteBuffers, 1, &m_buffer);
}

} // namespace engine::resources
