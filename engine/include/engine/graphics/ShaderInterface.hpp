/**
 * @file ShaderInterface.hpp
 * @brief Conventions and memory layouts shared between the engine and the GLSL shaders.
 *
 * Everything here has a GLSL counterpart in `resources/shaders/include/frame.glsl`; keep the two in sync.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <glm/glm.hpp>
#include <engine/resources/Texture.hpp>

namespace engine::graphics {

static constexpr std::size_t MAX_POINT_LIGHTS    = 4;
static constexpr std::size_t MAX_SHADOW_CASCADES = 4;

static constexpr std::string_view FRAME_UNIFORMS_BLOCK = "FrameUniforms";
static constexpr uint32_t FRAME_UNIFORMS_BINDING       = 0;

static constexpr std::string_view SHADOW_MAP_SAMPLER = "shadowMap";
static constexpr uint32_t SHADOW_MAP_TEXTURE_UNIT    = 7;

/**
 * @brief Every mesh texture type is always bound to the same texture unit,
 * so sampler uniforms are assigned once when a shader is linked instead of on every draw.
 */
constexpr uint32_t texture_unit(const resources::TextureType type) {
    switch (type) {
        case resources::TextureType::Specular: return 1;
        case resources::TextureType::Normal: return 2;
        case resources::TextureType::Height: return 3;
        default: return 0;
    }
}

/**
 * @brief std140 layout of a point light inside @ref FrameUniforms.
 */
struct PointLightUniforms {
    glm::vec3 position{};
    float constant{};
    glm::vec3 diffuse{};
    float linear{};
    glm::vec3 specular{};
    float quadratic{};
};

/**
 * @brief std140 layout of the per-frame uniform block. Uploaded once per frame by the @ref Renderer
 * and read by every scene shader that includes `frame.glsl`.
 */
struct FrameUniforms {
    glm::mat4 projection{};
    glm::mat4 view{};
    glm::mat4 light_space_matrices[MAX_SHADOW_CASCADES]{};
    glm::vec4 cascade_texel_sizes{};

    glm::vec3 view_position{};
    float time{};
    glm::vec3 light_position{};
    float shininess{};
    glm::vec3 light_ambient{};
    int32_t point_light_count{};
    glm::vec3 light_diffuse{};
    int32_t shadows_enabled{};
    glm::vec3 light_specular{};
    int32_t cascade_count{};
    glm::vec3 light_color{};
    int32_t fog_enabled{};
    glm::vec3 fog_color{};
    float fog_intensity{};
    float fog_start{};
    float fog_end{};
    float padding[2]{};

    PointLightUniforms point_lights[MAX_POINT_LIGHTS]{};
};

static_assert(sizeof(PointLightUniforms) == 48);
static_assert(MAX_SHADOW_CASCADES <= 4, "cascade_texel_sizes packs one float per cascade into a vec4");
static_assert(offsetof(FrameUniforms, view_position) == 400);
static_assert(offsetof(FrameUniforms, fog_start) == 512);
static_assert(offsetof(FrameUniforms, point_lights) == 528);

} // namespace engine::graphics
