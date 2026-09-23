#include <engine/resources/InstanceBuffer.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

    void Model::draw(const Shader *shader) const {
        shader->use();
        for (auto &mesh: m_meshes) {
            mesh.draw(shader);
        }
    }

    void Model::draw_instanced(const Shader *shader, const InstanceBuffer &instances, const uint32_t instance_count) const {
        shader->use();
        for (std::size_t i = 0; i < m_meshes.size(); ++i) {
            m_meshes[i].draw_instanced(shader, instances.vao(i), instance_count);
        }
    }

    void Model::destroy() const {
        for (auto &mesh: m_meshes) {
            mesh.destroy();
        }
    }
}
