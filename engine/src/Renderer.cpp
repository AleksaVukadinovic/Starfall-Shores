#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/util/Transform.hpp>

namespace engine::graphics {

void Renderer::initialize() {
    m_lighting = get<LightingController>();
    m_shadow = get<ShadowController>();
    m_graphics = get<GraphicsController>();
    m_camera = m_graphics->camera();
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

void Renderer::apply_effect(const resources::Shader *shader, const glm::mat4 &transform, const Effect effect) const {
    const auto time = static_cast<float>(platform::PlatformController::get_time());

    switch (effect) {
        case Effect::None:
            m_lighting->apply_to_shader(shader);
            shader->set_mat4("model", transform);
            break;
        case Effect::Wind:
            m_lighting->apply_to_shader(shader);
            shader->set_float("time", time);
            shader->set_bool("windEnabled", wind.enabled);
            shader->set_float("windIntensity", wind.intensity);
            shader->set_mat4("model", transform);
            break;
        case Effect::Water:
            m_lighting->apply_to_shader(shader);
            shader->set_float("time", time);
            shader->set_vec3("waterColor", water.color);
            shader->set_vec3("lightPos", m_lighting->light.position);
            shader->set_mat4("model", transform);
            break;
        case Effect::Fire:
            shader->use();
            shader->set_vec3("viewPos", m_camera->Position);
            shader->set_mat4("projection", m_graphics->projection_matrix());
            shader->set_mat4("view", m_camera->view_matrix());
            shader->set_mat4("model", transform);
            shader->set_float("time", static_cast<float>(platform::PlatformController::get_time() - fire.start_time));
            shader->set_vec3("fireColor", fire.fire_color);
            shader->set_vec3("glowColor", fire.glow_color);
            shader->set_float("intensity", fire.intensity);
            shader->set_float("flickerSpeed", fire.flicker_speed);
            shader->set_float("distortionAmount", fire.distortion_amount);
            break;
    }
}

void Renderer::draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, const Effect effect) const {
    apply_effect(shader, transform, effect);
    model->draw(shader);
}

void Renderer::draw(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, const Effect effect) const {
    draw(m_resources->model(model_name), m_resources->shader(shader_name), transform, effect);
}

void Renderer::draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, const Effect effect) const {
    apply_effect(shader, transform, effect);
    model->draw_blended(shader);
}

void Renderer::draw_blended(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, const Effect effect) const {
    draw_blended(m_resources->model(model_name), m_resources->shader(shader_name), transform, effect);
}

void Renderer::draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms, const Effect effect) const {
    apply_effect(shader, glm::mat4(1.0f), effect);
    model->draw_instanced(shader, transforms);
}

void Renderer::draw_instanced(const std::string &model_name, const std::string &shader_name, const std::vector<glm::mat4> &transforms, const Effect effect) const {
    draw_instanced(m_resources->model(model_name), m_resources->shader(shader_name), transforms, effect);
}

void Renderer::draw_batch_impl(const std::string &model_name, const std::string &shader_name,
                               const util::TransformData *transforms, const std::size_t count,
                               const DrawMethod method, const Effect effect) const {
    for (std::size_t i = 0; i < count; ++i) {
        const auto matrix = util::model_matrix(transforms[i]);
        switch (method) {
            case DrawMethod::Default:
                draw(model_name, shader_name, matrix, effect);
                break;
            case DrawMethod::Blended:
                draw_blended(model_name, shader_name, matrix, effect);
                break;
            case DrawMethod::Instanced:
                break;
        }
    }
}

void Renderer::draw_batch_instanced_impl(const std::string &model_name, const std::string &shader_name,
                                         const util::TransformData *transforms, const std::size_t count,
                                         const Effect effect) const {
    std::vector<glm::mat4> matrices;
    matrices.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        matrices.push_back(util::model_matrix(transforms[i]));
    draw_instanced(model_name, shader_name, matrices, effect);
}

}
