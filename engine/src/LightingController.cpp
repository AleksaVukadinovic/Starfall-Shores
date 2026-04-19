#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {

void LightingController::initialize() {
    m_graphics = get<GraphicsController>();
    m_camera = m_graphics->camera();
}

void LightingController::apply_to_shader(const resources::Shader *shader) const {
    shader->use();
    shader->set_mat4("projection", m_graphics->projection_matrix());
    shader->set_mat4("view", m_camera->view_matrix());
    shader->set_vec3("viewPos", m_camera->Position);

    shader->set_vec3("light.position", light.position);
    shader->set_vec3("light.ambient", light.ambient);
    shader->set_vec3("light.diffuse", light.diffuse);
    shader->set_vec3("light.specular", light.specular);
    shader->set_vec3("lightColor", light.color);
    shader->set_float("material.shininess", light.shininess);

    get<ShadowController>()->apply_to_shader(shader);
    get<FogController>()->apply_to_shader(shader);
}

}
