#include <engine/graphics/LightingController.hpp>
#include <engine/util/Errors.hpp>

namespace engine::graphics {

void LightingController::add_point_light(const PointLightSource &point_light) {
    RG_GUARANTEE(m_point_lights.size() < MAX_POINT_LIGHTS, "At most {} point lights are supported.", MAX_POINT_LIGHTS);
    m_point_lights.push_back(point_light);
}

void LightingController::clear_point_lights() {
    m_point_lights.clear();
}

}
