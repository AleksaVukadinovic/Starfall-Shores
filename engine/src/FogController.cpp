#include <engine/graphics/FogController.hpp>

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
