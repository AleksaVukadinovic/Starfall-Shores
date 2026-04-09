#pragma once

#include <glm/glm.hpp>

#include <engine/core/Controller.hpp>

class FogController : public engine::core::Controller {
public:
    bool fog_enabled    = false;
    float fog_intensity = 5.6f;
    float fog_start     = 22.0f;
    float fog_end       = 200.0f;
    glm::vec3 fog_color = glm::vec3(0.5f, 0.5f, 0.5f);

    [[nodiscard]] std::string_view name() const override {
        return "FogController";
    }
};
