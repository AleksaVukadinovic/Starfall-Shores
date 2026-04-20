/**
 * @file LightingController.hpp
 * @brief Provides a centralized light source and applies camera, lighting, shadow, and fog uniforms to shaders.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightSource.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {

/**
 * @struct Light
 * @brief Describes a single directional/point light source.
 */
struct Light {
    glm::vec3 position  = glm::vec3(0.0f, 60.0f, 0.0f);
    glm::vec3 ambient   = glm::vec3(0.2f);
    glm::vec3 diffuse   = glm::vec3(0.5f);
    glm::vec3 specular  = glm::vec3(0.1f);
    glm::vec3 color     = glm::vec3(1.0f);
    float shininess     = 1024.0f;
};

/**
 * @class LightingController
 * @brief Manages scene lighting and provides a single call to set all common shader uniforms.
 *
 * Combines camera matrices, light properties, shadow mapping, and fog into one
 * `apply_to_shader` call, replacing the boilerplate that every draw function would otherwise repeat.
 *
 * Register it in your App::app_setup:
 * @code
 * auto lighting = register_controller<engine::graphics::LightingController>();
 * lighting->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
 * @endcode
 *
 * Usage in a draw method:
 * @code
 * auto lighting = get<engine::graphics::LightingController>();
 * lighting->apply_to_shader(shader);  // sets view, projection, light, shadow, fog uniforms
 * @endcode
 */
class LightingController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "LightingController";
    }

    Light light;

    void add_point_light(const PointLightSource &point_light);
    void clear_point_lights();

    /**
     * @brief Activates the shader and sets all common uniforms: view, projection, viewPos,
     * light properties, material shininess, shadow mapping, fog, and point lights.
     */
    void apply_to_shader(const resources::Shader *shader) const;

    /**
     * @brief Applies lighting to a shader with per-object overrides from a LightSource.
     * Fields set in the override replace the corresponding global light values.
     */
    void apply_to_shader(const resources::Shader *shader, const LightSource &override) const;

private:
    GraphicsController *m_graphics = nullptr;
    Camera* m_camera = nullptr;
    std::vector<PointLightSource> m_point_lights;

    void apply_point_lights(const resources::Shader *shader) const;

    void initialize() override;
};

}
