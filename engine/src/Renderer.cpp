#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {

void Renderer::initialize() {
    m_lighting = get<LightingController>();
}

void Renderer::draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const {
    m_lighting->apply_to_shader(shader);
    shader->set_mat4("model", transform);
    model->draw(shader);
}

void Renderer::draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const {
    m_lighting->apply_to_shader(shader);
    shader->set_mat4("model", transform);
    model->draw_blended(shader);
}

void Renderer::draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms) const {
    m_lighting->apply_to_shader(shader);
    model->draw_instanced(shader, transforms);
}

}
