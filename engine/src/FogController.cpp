#include <engine/graphics/FogController.hpp>
#include <engine/resources/Shader.hpp>

void FogController::apply_to_shader(const engine::resources::Shader *shader) const {
    shader->set_bool("fogEnabled", fog_enabled);
    shader->set_float("fogIntensity", fog_intensity);
    shader->set_float("fogStart", fog_start);
    shader->set_float("fogEnd", fog_end);
    shader->set_vec3("fogColor", fog_color);
}

void FogController::transition(const float progress, const bool currently_day) {
    const auto start_color  = currently_day ? FOG_COLOR_DAY : FOG_COLOR_NIGHT;
    const auto target_color = currently_day ? FOG_COLOR_NIGHT : FOG_COLOR_DAY;
    fog_color = glm::mix(start_color, target_color, progress);
}

void FogController::set_day(const bool is_day) {
    fog_color = is_day ? FOG_COLOR_DAY : FOG_COLOR_NIGHT;
}

float FogController::far_plane() const {
    return fog_enabled ? FAR_PLANE_FOG : FAR_PLANE_CLEAR;
}
