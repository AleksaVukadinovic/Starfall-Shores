#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/resources/Shader.hpp>
#include <format>

namespace engine::graphics {

void LightingController::initialize() {
    m_graphics = get<GraphicsController>();
    m_camera = m_graphics->camera();
}

void LightingController::add_point_light(const PointLightSource &point_light) {
    m_point_lights.push_back(point_light);
}

void LightingController::clear_point_lights() {
    m_point_lights.clear();
}

void LightingController::apply_point_lights(const resources::Shader *shader) const {
    const auto count = static_cast<int>(m_point_lights.size());
    shader->set_int("numPointLights", count);
    for (int i = 0; i < count; ++i) {
        const auto &pl = m_point_lights[i];
        const auto prefix = std::format("pointLights[{}]", i);
        shader->set_vec3(prefix + ".position", pl.position.value_or(glm::vec3(0.0f)));
        shader->set_vec3(prefix + ".diffuse", pl.diffuse.value_or(glm::vec3(1.0f)));
        shader->set_vec3(prefix + ".specular", pl.specular.value_or(glm::vec3(1.0f)));
        shader->set_float(prefix + ".constant", pl.constant);
        shader->set_float(prefix + ".linear", pl.linear);
        shader->set_float(prefix + ".quadratic", pl.quadratic);
    }
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
    apply_point_lights(shader);
}

void LightingController::apply_to_shader(const resources::Shader *shader, const LightSource &override) const {
    apply_to_shader(shader);

    if (override.position)
        shader->set_vec3("light.position", *override.position);
    if (override.ambient)
        shader->set_vec3("light.ambient", *override.ambient);
    if (override.diffuse)
        shader->set_vec3("light.diffuse", *override.diffuse);
    if (override.specular)
        shader->set_vec3("light.specular", *override.specular);
    if (override.color)
        shader->set_vec3("lightColor", *override.color);
    if (override.shininess)
        shader->set_float("material.shininess", *override.shininess);
}

}
