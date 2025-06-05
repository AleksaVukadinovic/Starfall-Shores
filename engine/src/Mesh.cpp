#include<glad/glad.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/util/Utils.hpp>
#include <engine/resources/Mesh.hpp>
#include <engine/resources/Shader.hpp>
#include <unordered_map>

namespace engine::resources {

    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
               std::vector<Texture *> textures) {
        // NOLINTBEGIN
        static_assert(std::is_trivial_v<Vertex>);
        uint32_t VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

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

        glBindVertexArray(0);
        // NOLINTEND
        m_vao         = VAO;
        m_num_indices = indices.size();
        m_textures    = std::move(textures);
    }

    void Mesh::draw(const Shader *shader) {
        std::unordered_map<std::string_view, uint32_t> counts;
        std::string uniform_name;
        uniform_name.reserve(32);
        for (int i = 0; i < m_textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            const auto &texture_type = Texture::uniform_name_convention(m_textures[i]->type());
            uniform_name.append(texture_type);
            const auto count = (counts[texture_type] += 1);
            uniform_name.append(std::to_string(count));
            shader->set_int(uniform_name, i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]->id());
            uniform_name.clear();
        }
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Mesh::drawInstanced(const Shader *shader, const unsigned int number_of_instances) {
        std::unordered_map<std::string_view, uint32_t> counts;
        std::string uniform_name;
        uniform_name.reserve(32);
        for (int i = 0; i < m_textures.size(); i++) {
            CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + i);
            const auto &texture_type = Texture::uniform_name_convention(m_textures[i]->type());
            uniform_name.append(texture_type);
            const auto count = (counts[texture_type] += 1);
            uniform_name.append(std::to_string(count));
            shader->set_int(uniform_name, i);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_textures[i]->id());
            uniform_name.clear();
        }
        CHECKED_GL_CALL(glBindVertexArray, m_vao);
        CHECKED_GL_CALL(glDrawElementsInstanced, GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, nullptr, number_of_instances);
        CHECKED_GL_CALL(glBindVertexArray, 0);
    }

    void Mesh::destroy() {
        glDeleteVertexArrays(1, &m_vao);
    }

}
