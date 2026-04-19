#include <engine/graphics/ShadowController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {

void ShadowController::initialize() {
    auto [fbo, texture] = OpenGL::create_shadow_map(map_size, map_size);
    m_fbo = fbo;
    m_texture = texture;
}

void ShadowController::terminate() {
    if (m_fbo) {
        OpenGL::destroy_shadow_map({m_fbo, m_texture});
        m_fbo = 0;
        m_texture = 0;
    }
}

glm::mat4 ShadowController::light_space_matrix() const {
    const auto projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near_plane, far_plane);
    const auto view = glm::lookAt(light_position, light_target, light_up);
    return projection * view;
}

void ShadowController::begin_depth_pass() const {
    OpenGL::begin_shadow_pass(m_fbo, map_size, map_size);
}

void ShadowController::end_depth_pass() const {
    const auto platform = get<platform::PlatformController>();
    OpenGL::end_shadow_pass(platform->window()->width(), platform->window()->height());
}

void ShadowController::apply_to_shader(const resources::Shader *shader) const {
    shader->set_bool("shadowsEnabled", enabled);
    if (enabled) {
        shader->set_mat4("lightSpaceMatrix", light_space_matrix());
        shader->set_int("shadowMap", static_cast<int>(shadow_map_texture_unit));
        OpenGL::bind_texture_to_unit(m_texture, shadow_map_texture_unit);
    }
}

void ShadowController::setup_depth_shader(const resources::Shader *shader) const {
    shader->use();
    shader->set_mat4("lightSpaceMatrix", light_space_matrix());
}

void ShadowController::setup_depth_instanced_shader(const resources::Shader *shader) const {
    shader->use();
    shader->set_mat4("lightSpaceMatrix", light_space_matrix());
}

}
