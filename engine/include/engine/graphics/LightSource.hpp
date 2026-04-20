#pragma once
#include <glm/glm.hpp>
#include <optional>

namespace engine::graphics {

struct LightSource {
    std::optional<glm::vec3> position;
    std::optional<glm::vec3> ambient;
    std::optional<glm::vec3> diffuse;
    std::optional<glm::vec3> specular;
    std::optional<glm::vec3> color;
    std::optional<float> shininess;
};

struct PointLightSource : LightSource {
    float constant  = 1.0f;
    float linear    = 0.09f;
    float quadratic = 0.032f;
};

struct DirectionalLightSource : LightSource {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
};

struct SpotlightSource : LightSource {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    float cutOff      = 12.5f;
    float outerCutOff = 17.5f;
    float constant    = 1.0f;
    float linear      = 0.09f;
    float quadratic   = 0.032f;
};

}
