/**
 * @file Renderer.hpp
 * @brief Provides a high-level rendering API that combines lighting, shadows, fog, and model drawing.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace engine::resources {
class Model;
class Shader;
}

namespace engine::graphics {
class LightingController;

/**
 * @class Renderer
 * @brief High-level rendering interface that applies lighting/shadow/fog uniforms
 * and dispatches draw calls with the appropriate technique.
 *
 * Eliminates the boilerplate of manually calling LightingController::apply_to_shader,
 * setting the model matrix uniform, and choosing the right Model::draw variant.
 *
 * Register it in your App::app_setup:
 * @code
 * auto renderer = register_controller<engine::graphics::Renderer>();
 * renderer->after(engine::core::Controller::get<engine::graphics::LightingController>());
 * @endcode
 *
 * Usage:
 * @code
 * auto renderer = get<engine::graphics::Renderer>();
 * renderer->draw(model, shader, transform);
 * renderer->draw_blended(model, shader, transform);
 * renderer->draw_instanced(model, shader, matrices);
 * @endcode
 */
class Renderer final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "Renderer";
    }

    /**
     * @brief Applies lighting uniforms, sets the model matrix, and draws the model.
     */
    void draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const;

    /**
     * @brief Applies lighting uniforms, sets the model matrix, and draws the model with alpha blending enabled.
     */
    void draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const;

    /**
     * @brief Applies lighting uniforms and draws the model using hardware instancing.
     */
    void draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms) const;

private:
    void initialize() override;

    LightingController *m_lighting = nullptr;
};

}
