#include <glad/glad.h>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

    void Model::draw(const Shader *shader) {
        shader->use();
        for (auto &mesh: m_meshes) {
            mesh.draw(shader);
        }
    }

    void Model::drawBlended(const Shader *shader) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        draw(shader);
        glDisable(GL_BLEND);
    }

    void Model::drawInstanced(const Shader *shader, const unsigned int number_of_instances, glm::mat4* model_matrices) {
        unsigned int buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, number_of_instances * sizeof(glm::mat4), model_matrices, GL_STATIC_DRAW);

        for (const auto &mesh: this->m_meshes) {
            const unsigned int VAO = mesh.m_vao;
            glBindVertexArray(VAO);

            constexpr std::size_t size = sizeof(glm::vec4);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * size, (void *) 0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * size, (void *) (size));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * size, (void *) (2 * size));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * size, (void *) (3 * size));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }

        shader->use();
        for (auto &mesh: m_meshes) {
            mesh.drawInstanced(shader, number_of_instances);
        }
    }

    void Model::destroy() {
        for (auto &mesh: m_meshes) {
            mesh.destroy();
        }
    }
}
