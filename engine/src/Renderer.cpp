#include <glad/glad.h>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/ShaderInterface.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {

void Renderer::initialize() {
    m_lighting = get<LightingController>();
    m_shadow = get<ShadowController>();
    m_graphics = get<GraphicsController>();
    m_fog = get<FogController>();
    m_rain = get<RainController>();
    m_camera = m_graphics->camera();
    const auto resources = get<resources::ResourcesController>();
    m_depth_shader = resources->shader("depth_shader");
    m_depth_instanced_shader = resources->shader("depth_instanced_shader");

    CHECKED_GL_CALL(glGenBuffers, 1, &m_frame_uniform_buffer);
    CHECKED_GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, m_frame_uniform_buffer);
    CHECKED_GL_CALL(glBufferData, GL_UNIFORM_BUFFER, sizeof(FrameUniforms), nullptr, GL_DYNAMIC_DRAW);
    CHECKED_GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, FRAME_UNIFORMS_BINDING, m_frame_uniform_buffer);
}

void Renderer::terminate() {
    for (const auto &batch: m_batches) {
        batch->destroy();
    }
    m_batches.clear();
    CHECKED_GL_CALL(glDeleteBuffers, 1, &m_frame_uniform_buffer);
}

void Renderer::begin_draw() {
    m_stats = {};
    render_shadow_maps();
    update_frame_uniforms();
    m_view_frustum = util::Frustum(m_graphics->projection_matrix() * m_camera->view_matrix());
    m_shadow->bind_shadow_map();
}

void Renderer::set_depth_scene(std::function<void()> callback) {
    m_depth_scene_callback = std::move(callback);
}

const DrawBatch *Renderer::create_batch(const resources::Model *model, const std::span<const util::TransformData> transforms,
                                        const DrawMethod method) {
    // DrawBatch has a private constructor, so it can't go through std::make_unique.
    return m_batches.emplace_back(new DrawBatch(model, transforms, method)).get();
}

void Renderer::render_shadow_maps() {
    if (!m_shadow->enabled || !m_depth_scene_callback) {
        return;
    }
    bool any_rendered = false;
    for (std::size_t cascade = 0; cascade < ShadowController::CASCADE_COUNT; ++cascade) {
        if (!m_shadow->is_stale(cascade)) {
            continue;
        }
        if (!any_rendered) {
            m_graphics->profiler().begin_section("Shadows");
            any_rendered = true;
        }
        const glm::mat4 &light_space = m_shadow->light_space_matrix(cascade);
        m_shadow_frustum = util::Frustum(light_space);
        m_depth_shader->use();
        m_depth_shader->set_mat4("lightSpaceMatrix", light_space);
        m_depth_instanced_shader->use();
        m_depth_instanced_shader->set_mat4("lightSpaceMatrix", light_space);

        m_shadow->begin_depth_pass(cascade);
        m_depth_scene_callback();
    }
    if (any_rendered) {
        m_shadow->end_depth_pass();
    }
}

void Renderer::update_frame_uniforms() const {
    FrameUniforms uniforms;
    uniforms.projection = m_graphics->projection_matrix();
    uniforms.view       = m_camera->view_matrix();
    for (std::size_t cascade = 0; cascade < ShadowController::CASCADE_COUNT; ++cascade) {
        uniforms.light_space_matrices[cascade] = m_shadow->light_space_matrix(cascade);
        uniforms.cascade_texel_sizes[static_cast<int>(cascade)] = ShadowController::texel_size(cascade);
    }
    uniforms.cascade_count   = ShadowController::CASCADE_COUNT;
    uniforms.shadows_enabled = m_shadow->enabled;

    uniforms.view_position = m_camera->Position;
    uniforms.time          = static_cast<float>(platform::PlatformController::get_time());

    const Light &light      = m_lighting->light;
    uniforms.light_position = light.position;
    uniforms.light_ambient  = light.ambient;
    uniforms.light_diffuse  = light.diffuse;
    uniforms.light_specular = light.specular;
    uniforms.light_color    = light.color;
    uniforms.shininess      = light.shininess;

    const auto point_lights    = m_lighting->point_lights();
    uniforms.point_light_count = static_cast<int32_t>(point_lights.size());
    for (std::size_t i = 0; i < point_lights.size(); ++i) {
        const PointLightSource &source = point_lights[i];
        uniforms.point_lights[i] = {
                .position = source.position.value_or(glm::vec3(0.0f)),
                .constant = source.constant,
                .diffuse = source.diffuse.value_or(glm::vec3(1.0f)),
                .linear = source.linear,
                .specular = source.specular.value_or(glm::vec3(1.0f)),
                .quadratic = source.quadratic,
        };
    }

    uniforms.fog_enabled   = m_fog->fog_enabled;
    uniforms.fog_color     = m_fog->fog_color;
    uniforms.fog_intensity = m_fog->fog_intensity;
    uniforms.fog_start     = m_fog->fog_start;
    uniforms.fog_end       = m_fog->fog_end;

    uniforms.wetness        = m_rain->wetness();
    uniforms.rain_intensity = m_rain->current_intensity();

    CHECKED_GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, m_frame_uniform_buffer);
    CHECKED_GL_CALL(glBufferSubData, GL_UNIFORM_BUFFER, 0, sizeof(FrameUniforms), &uniforms);
}

void Renderer::apply_effect(const resources::Shader *shader, const Effect effect) const {
    switch (effect) {
        case Effect::None:
            break;
        case Effect::Wind:
            shader->set_bool("windEnabled", wind.enabled);
            shader->set_float("windIntensity", wind.intensity);
            break;
        case Effect::Water:
            shader->set_vec3("waterColor", water.color);
            break;
        case Effect::Fire:
            shader->set_float("flameTime", static_cast<float>(platform::PlatformController::get_time() - fire.start_time));
            shader->set_vec3("fireColor", fire.fire_color);
            shader->set_vec3("glowColor", fire.glow_color);
            shader->set_float("intensity", fire.intensity);
            shader->set_float("flickerSpeed", fire.flicker_speed);
            shader->set_float("distortionAmount", fire.distortion_amount);
            break;
    }
}

float Renderer::bounds_padding(const Effect effect) {
    return effect == Effect::None ? 0.0f : ANIMATED_BOUNDS_PADDING;
}

std::span<const glm::mat4> Renderer::visible_transforms(const DrawBatch &batch, const util::Frustum &frustum, const float padding) {
    m_visible_transforms.clear();
    const auto bounds = batch.bounds();
    for (std::size_t i = 0; i < bounds.size(); ++i) {
        if (frustum.intersects(padding > 0.0f ? bounds[i].padded(padding) : bounds[i])) {
            m_visible_transforms.push_back(batch.transforms()[i]);
        }
    }
    return m_visible_transforms;
}

void Renderer::draw(const DrawBatch *batch, const resources::Shader *shader, const Effect effect) {
    shader->use();
    apply_effect(shader, effect);
    const auto visible      = visible_transforms(*batch, m_view_frustum, bounds_padding(effect));
    const auto *model       = batch->model();
    const auto visible_count = static_cast<uint32_t>(visible.size());
    m_stats.objects_drawn += visible_count;
    m_stats.objects_culled += static_cast<uint32_t>(batch->transforms().size()) - visible_count;
    if (visible.empty()) {
        return;
    }

    if (batch->method() == DrawMethod::Instanced) {
        batch->m_instances->upload(visible);
        model->draw_instanced(shader, *batch->m_instances, visible_count);
        m_stats.draw_calls += static_cast<uint32_t>(model->meshes().size());
        return;
    }

    const bool blended = batch->method() == DrawMethod::Blended;
    if (blended) {
        OpenGL::enable_blending();
    }
    for (const auto &transform: visible) {
        shader->set_mat4("model", transform);
        model->draw(shader);
    }
    if (blended) {
        OpenGL::disable_blending();
    }
    m_stats.draw_calls += visible_count * static_cast<uint32_t>(model->meshes().size());
}

void Renderer::draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, const Effect effect) {
    if (!m_view_frustum.intersects(model->bounds().transformed(transform).padded(bounds_padding(effect)))) {
        ++m_stats.objects_culled;
        return;
    }
    shader->use();
    apply_effect(shader, effect);
    shader->set_mat4("model", transform);
    model->draw(shader);
    ++m_stats.objects_drawn;
    m_stats.draw_calls += static_cast<uint32_t>(model->meshes().size());
}

void Renderer::draw_depth(const DrawBatch *batch) {
    const auto visible = visible_transforms(*batch, m_shadow_frustum, 0.0f);
    if (visible.empty()) {
        return;
    }
    const auto *model = batch->model();
    if (batch->method() == DrawMethod::Instanced) {
        batch->m_instances->upload(visible);
        model->draw_instanced(m_depth_instanced_shader, *batch->m_instances, static_cast<uint32_t>(visible.size()));
        return;
    }
    m_depth_shader->use();
    for (const auto &transform: visible) {
        m_depth_shader->set_mat4("model", transform);
        model->draw(m_depth_shader);
    }
}

}
