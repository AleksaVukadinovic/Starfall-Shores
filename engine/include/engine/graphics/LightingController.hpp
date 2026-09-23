/**
 * @file LightingController.hpp
 * @brief Holds the scene light sources. The @ref Renderer uploads them to the GPU once per frame.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/graphics/LightSource.hpp>
#include <engine/graphics/ShaderInterface.hpp>
#include <glm/glm.hpp>
#include <span>
#include <vector>

namespace engine::graphics {

/**
 * @struct Light
 * @brief Describes the main light source of the scene (the sun or the moon).
 */
struct Light {
    glm::vec3 position  = glm::vec3(0.0f, 60.0f, 0.0f);
    glm::vec3 ambient   = glm::vec3(0.2f);
    glm::vec3 diffuse   = glm::vec3(0.5f);
    glm::vec3 specular  = glm::vec3(0.1f);
    glm::vec3 color     = glm::vec3(1.0f);
    float shininess     = 1024.0f;
};

class LightingController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "LightingController";
    }

    Light light;

    /**
     * @brief Adds a point light. At most @ref MAX_POINT_LIGHTS point lights are supported.
     */
    void add_point_light(const PointLightSource &point_light);

    void clear_point_lights();

    [[nodiscard]] std::span<const PointLightSource> point_lights() const {
        return m_point_lights;
    }

private:
    std::vector<PointLightSource> m_point_lights;
};

}
