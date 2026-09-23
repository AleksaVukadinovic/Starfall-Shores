#include<glad/glad.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/ShaderInterface.hpp>
#include <engine/util/Errors.hpp>
#include <engine/resources/Mesh.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
               const std::vector<Texture *> &textures) {
        static_assert(std::is_trivial_v<Vertex>);
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
        attach_vertex_buffers();
        glBindVertexArray(0);

        m_index_count = indices.size();
        for (const Texture *texture: textures) {
            const uint32_t unit = graphics::texture_unit(texture->type());
            RG_GUARANTEE(unit < TEXTURE_UNIT_COUNT, "Texture unit {} is out of the range reserved for mesh textures.", unit);
            if (m_texture_ids[unit] == 0) {
                m_texture_ids[unit] = texture->id();
            }
            m_has_normal_map |= texture->type() == TextureType::Normal;
        }
    }

    void Mesh::attach_vertex_buffers() const {
        // NOLINTBEGIN
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Bitangent));
        // NOLINTEND
    }

    void Mesh::bind_textures(const Shader *shader) const {
        for (uint32_t unit = 0; unit < TEXTURE_UNIT_COUNT; ++unit) {
            if (m_texture_ids[unit] != 0) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, m_texture_ids[unit]);
            }
        }
        shader->set_bool("hasNormalMap", m_has_normal_map);
    }

    void Mesh::draw(const Shader *shader) const {
        bind_textures(shader);
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, nullptr);
    }

    void Mesh::draw_instanced(const Shader *shader, const uint32_t instanced_vao, const uint32_t instance_count) const {
        bind_textures(shader);
        CHECKED_GL_CALL(glBindVertexArray, instanced_vao);
        CHECKED_GL_CALL(glDrawElementsInstanced, GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, nullptr, instance_count);
    }

    void Mesh::destroy() const {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }

}
