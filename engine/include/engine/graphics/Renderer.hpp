/**
 * @file Renderer.hpp
 * @brief Provides a high-level rendering API that combines lighting, shadows, fog, and model drawing.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <vector>

namespace engine::resources {
class Model;
class Shader;
class ResourcesController;
}

namespace engine::graphics {
class LightingController;
class ShadowController;

/**
 * @class Renderer
 * @brief High-level rendering interface that applies lighting/shadow/fog uniforms
 * and dispatches draw calls with the appropriate technique.
 *
 * Also orchestrates the shadow depth pass automatically via a user-provided callback.
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
 * renderer->set_depth_scene([this] { render_depth_scene(); });
 * // in draw:
 * renderer->draw("terrain", "basic", transform);
 * renderer->draw_blended("water", "water_shader", transform);
 * renderer->draw_instanced("path", "flower_shader", matrices);
 * @endcode
 */
class Renderer final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "Renderer";
    }

    void draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const;
    void draw(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform) const;

    void draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform) const;
    void draw_blended(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform) const;

    void draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms) const;
    void draw_instanced(const std::string &model_name, const std::string &shader_name, const std::vector<glm::mat4> &transforms) const;

    /**
     * @brief Registers a callback that renders the scene for the shadow depth pass.
     * The Renderer calls this automatically in begin_draw when shadows are enabled.
     */
    void set_depth_scene(std::function<void()> callback);

private:
    void initialize() override;
    void begin_draw() override;

    LightingController *m_lighting = nullptr;
    ShadowController *m_shadow = nullptr;
    resources::ResourcesController *m_resources = nullptr;
    std::function<void()> m_depth_scene_callback;
};

}
