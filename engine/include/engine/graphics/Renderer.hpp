#pragma once

#include <engine/core/Controller.hpp>
#include <engine/graphics/DrawBatch.hpp>
#include <engine/util/Geometry.hpp>
#include <engine/util/Transform.hpp>
#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

class FogController;
class RainController;
class SnowController;

namespace engine::resources {
class Model;
class Shader;
}

namespace engine::graphics {
class LightingController;
class WaterController;
class ShadowController;
class GraphicsController;
class Camera;

struct WindParams {
    bool enabled       = false;
    float intensity    = 0.8f;
};

struct FireParams {
    glm::vec3 fire_color        = glm::vec3(1.0f, 0.6f, 0.2f);
    glm::vec3 glow_color        = glm::vec3(1.0f, 0.3f, 0.0f);
    float intensity             = 50.0f;
    float flicker_speed         = 5.0f;
    float distortion_amount     = 0.1f;
    double start_time           = 0.0;
};

/**
 * @brief What the @ref Renderer drew in the current frame, excluding shadow passes.
 */
struct RenderStats {
    uint32_t draw_calls{0};
    uint32_t objects_drawn{0};
    uint32_t objects_culled{0};
};

/**
 * @class Renderer
 * @brief Draws @ref DrawBatch objects with frustum culling, and renders the shadow maps.
 *
 * Once per frame, before anything is drawn, the renderer:
 * 1. renders the stale shadow cascades by invoking the depth scene callback (see @ref Renderer::set_depth_scene),
 * 2. uploads camera, light, fog, rain, underwater and shadow data into the frame uniform block (see @ref FrameUniforms),
 *    so draws only set per-object uniforms.
 */
class Renderer final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "Renderer";
    }

    WindParams wind;
    FireParams fire;

    /**
     * @brief Creates a batch of `model` placed at `transforms`. The batch lives until the renderer terminates.
     */
    const DrawBatch *create_batch(const resources::Model *model, std::span<const util::TransformData> transforms,
                                  DrawMethod method = DrawMethod::Default);

    /**
     * @brief Draws every visible transform of `batch`.
     */
    void draw(const DrawBatch *batch, const resources::Shader *shader, Effect effect = Effect::None);

    /**
     * @brief Draws a single model at a transform that may change every frame, e.g. an object being placed in the editor.
     */
    void draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, Effect effect = Effect::None);

    /**
     * @brief Draws `model` once per transform with instancing, for transforms that change every frame (e.g. a school of fish).
     * The shader must read the instance matrix from attribute 5. Instances aren't culled, so their order is stable
     * (shaders can use gl_InstanceID to tell them apart).
     */
    void draw_instances(const resources::Model *model, const resources::Shader *shader, std::span<const glm::mat4> transforms);

    /**
     * @brief Sets the function that draws the shadow casters with @ref Renderer::draw_depth. It's called once per stale shadow cascade.
     */
    void set_depth_scene(std::function<void()> callback);

    /**
     * @brief Draws the batch into the shadow cascade that's currently being rendered. Call only from the depth scene callback.
     */
    void draw_depth(const DrawBatch *batch);

    [[nodiscard]] const RenderStats &stats() const {
        return m_stats;
    }

private:
    /**
     * @brief Bounds of objects whose vertices an effect animates are grown by this fraction, so they aren't culled while still visible.
     */
    static constexpr float ANIMATED_BOUNDS_PADDING = 0.25f;

    void initialize() override;
    void begin_draw() override;
    void terminate() override;

    void render_shadow_maps();
    void update_frame_uniforms() const;
    void apply_effect(const resources::Shader *shader, Effect effect) const;

    /**
     * @brief Collects the transforms of `batch` whose bounds (grown by `padding`) intersect `frustum`.
     */
    std::span<const glm::mat4> visible_transforms(const DrawBatch &batch, const util::Frustum &frustum, float padding);

    [[nodiscard]] static float bounds_padding(Effect effect);

    LightingController *m_lighting = nullptr;
    ShadowController *m_shadow = nullptr;
    GraphicsController *m_graphics = nullptr;
    FogController *m_fog = nullptr;
    RainController *m_rain = nullptr;
    SnowController *m_snow = nullptr;
    WaterController *m_water = nullptr;
    Camera *m_camera = nullptr;
    resources::Shader *m_depth_shader = nullptr;
    resources::Shader *m_depth_instanced_shader = nullptr;
    std::function<void()> m_depth_scene_callback;

    std::vector<std::unique_ptr<DrawBatch>> m_batches;
    /**
     * @brief Instance buffers for @ref Renderer::draw_instances, one per model, created on first use.
     */
    std::unordered_map<const resources::Model *, std::unique_ptr<resources::InstanceBuffer>> m_dynamic_instances;
    std::vector<glm::mat4> m_visible_transforms;
    util::Frustum m_view_frustum;
    util::Frustum m_shadow_frustum;
    uint32_t m_frame_uniform_buffer = 0;
    RenderStats m_stats;
};

}
