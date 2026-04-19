#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {

void Renderer::initialize() {
    m_lighting = get<LightingController>();
    m_shadow = get<ShadowController>();
    m_resources = get<resources::ResourcesController>();
}

void Renderer::begin_draw() {
    if (m_shadow->enabled && m_depth_scene_callback) {
        m_shadow->begin_depth_pass();
        m_depth_scene_callback();
        m_shadow->end_depth_pass();
    }
}

void Renderer::set_depth_scene(std::function<void()> callback) {
    m_depth_scene_callback = std::move(callback);
}

void Renderer::draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const {
    m_lighting->apply_to_shader(shader);
    shader->set_mat4("model", transform);
    model->draw(shader);
}

void Renderer::draw(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform) const {
    draw(m_resources->model(model_name), m_resources->shader(shader_name), transform);
}

void Renderer::draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const {
    m_lighting->apply_to_shader(shader);
    shader->set_mat4("model", transform);
    model->draw_blended(shader);
}

void Renderer::draw_blended(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform) const {
    draw_blended(m_resources->model(model_name), m_resources->shader(shader_name), transform);
}

void Renderer::draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms) const {
    m_lighting->apply_to_shader(shader);
    model->draw_instanced(shader, transforms);
}

void Renderer::draw_instanced(const std::string &model_name, const std::string &shader_name, const std::vector<glm::mat4> &transforms) const {
    draw_instanced(m_resources->model(model_name), m_resources->shader(shader_name), transforms);
}

}
