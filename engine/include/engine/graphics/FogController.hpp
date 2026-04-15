#pragma once

#include <glm/glm.hpp>

#include <engine/core/Controller.hpp>

namespace engine::resources {
    class Shader;
}

class FogController : public engine::core::Controller {
public:
    static constexpr float FAR_PLANE_CLEAR = 250.0f;
    static constexpr float FAR_PLANE_FOG   = 150.0f;
    static constexpr auto FOG_COLOR_DAY    = glm::vec3(0.6f, 0.6f, 0.6f);
    static constexpr auto FOG_COLOR_NIGHT  = glm::vec3(0.3f, 0.3f, 0.3f);

    bool fog_enabled    = false;
    float fog_intensity = 5.6f;
    float fog_start     = 22.0f;
    float fog_end       = FAR_PLANE_FOG;
    glm::vec3 fog_color = FOG_COLOR_DAY;

    void apply_to_shader(const engine::resources::Shader *shader) const;
    void transition(float progress, bool currently_day);
    void set_day(bool is_day);
    [[nodiscard]] float far_plane() const;

    [[nodiscard]] std::string_view name() const override {
        return "FogController";
    }
};
