/**
 * @file ShadowController.hpp
 * @brief Provides shadow mapping functionality with configurable light-space parameters.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {

/**
 * @class ShadowController
 * @brief Manages shadow map FBO lifecycle, depth pass control, and shadow uniform application.
 *
 * Register it in your App::app_setup to enable shadow mapping:
 * @code
 * auto shadows = register_controller<engine::graphics::ShadowController>();
 * shadows->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
 * @endcode
 *
 * Typical usage in a draw method:
 * @code
 * auto shadows = get<engine::graphics::ShadowController>();
 * if (shadows->enabled) {
 *     shadows->begin_depth_pass();
 *     // draw scene for depth
 *     shadows->end_depth_pass();
 * }
 * // later, when setting up a shader:
 * shadows->apply_to_shader(shader);
 * @endcode
 */
class ShadowController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "ShadowController";
    }

    bool enabled = true;

    uint32_t map_size = 4096;
    float ortho_size  = 120.0f;
    float near_plane  = 0.1f;
    float far_plane   = 200.0f;

    glm::vec3 light_position = glm::vec3(0.0f, 60.0f, 0.0f);
    glm::vec3 light_target   = glm::vec3(0.0f, 15.0f, 0.0f);
    glm::vec3 light_up       = glm::vec3(0.0f, 0.0f, -1.0f);

    uint32_t shadow_map_texture_unit = 7;

    [[nodiscard]] glm::mat4 light_space_matrix() const;

    void begin_depth_pass() const;
    void end_depth_pass() const;

    void apply_to_shader(const resources::Shader *shader) const;

    void setup_depth_shader(const resources::Shader *shader) const;
    void setup_depth_instanced_shader(const resources::Shader *shader) const;

private:
    void initialize() override;
    void terminate() override;

    uint32_t m_fbo     = 0;
    uint32_t m_texture = 0;
};

}
