/**
 * @file Renderer.hpp
 * @brief Provides a high-level rendering API that combines lighting, shadows, fog, and model drawing.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <engine/graphics/LightSource.hpp>
#include <engine/util/Transform.hpp>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <vector>

namespace engine::resources {
class Model;
class Shader;
class ResourcesController;
}

namespace engine::graphics {
class LightingController;
class ShadowController;
class GraphicsController;
class Camera;

enum class Effect {
    None,
    Wind,
    Water,
    Fire
};

enum class DrawMethod {
    Default,
    Blended,
    Instanced
};

struct WindParams {
    bool enabled       = false;
    float intensity    = 0.8f;
};

struct WaterParams {
    glm::vec3 color = glm::vec3(0.0f, 0.4f, 0.6f);
};

struct FireParams {
    glm::vec3 fire_color        = glm::vec3(1.0f, 0.6f, 0.2f);
    glm::vec3 glow_color        = glm::vec3(1.0f, 0.3f, 0.0f);
    float intensity             = 50.0f;
    float flicker_speed         = 5.0f;
    float distortion_amount     = 0.1f;
    double start_time           = 0.0;
};

class Renderer final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "Renderer";
    }

    WindParams wind;
    WaterParams water;
    FireParams fire;

    void draw(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, Effect effect = Effect::None) const;
    void draw(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, Effect effect = Effect::None) const;
    void draw(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, const LightSource &light_override, Effect effect = Effect::None) const;

    void draw_blended(const resources::Model *model, const resources::Shader *shader, const glm::mat4 &transform, Effect effect = Effect::None) const;
    void draw_blended(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, Effect effect = Effect::None) const;
    void draw_blended(const std::string &model_name, const std::string &shader_name, const glm::mat4 &transform, const LightSource &light_override, Effect effect = Effect::None) const;

    void draw_instanced(const resources::Model *model, const resources::Shader *shader, const std::vector<glm::mat4> &transforms, Effect effect = Effect::None) const;
    void draw_instanced(const std::string &model_name, const std::string &shader_name, const std::vector<glm::mat4> &transforms, Effect effect = Effect::None) const;

    void set_depth_scene(std::function<void()> callback);

    void draw_depth(const std::string &model_name, const glm::mat4 &transform) const;

    template<std::size_t N>
    void draw_depth_batch(const std::string &model_name,
                          const std::array<util::TransformData, N> &transforms) const {
        draw_depth_batch_impl(model_name, transforms.data(), transforms.size());
    }

    template<std::size_t N>
    void draw_depth_batch_instanced(const std::string &model_name,
                                    const std::array<util::TransformData, N> &transforms) const {
        draw_depth_batch_instanced_impl(model_name, transforms.data(), transforms.size());
    }

    void setup_depth_shader() const;
    void setup_depth_instanced_shader() const;

    template<std::size_t N>
    void draw_batch(const std::string &model_name, const std::string &shader_name,
                    const std::array<util::TransformData, N> &transforms,
                    const DrawMethod method = DrawMethod::Default, const Effect effect = Effect::None) {
        draw_batch_impl(model_name, shader_name, transforms.data(), transforms.size(), method, effect);
    }

    template<std::size_t N>
    void draw_batch_instanced(const std::string &model_name, const std::string &shader_name,
                              const std::array<util::TransformData, N> &transforms,
                              const Effect effect = Effect::None) {
        draw_batch_instanced_impl(model_name, shader_name, transforms.data(), transforms.size(), effect);
    }

private:
    void draw_batch_impl(const std::string &model_name, const std::string &shader_name,
                         const util::TransformData *transforms, std::size_t count,
                         DrawMethod method, Effect effect) const;
    void draw_batch_instanced_impl(const std::string &model_name, const std::string &shader_name,
                                   const util::TransformData *transforms, std::size_t count,
                                   Effect effect) const;

    void draw_depth_batch_impl(const std::string &model_name,
                               const util::TransformData *transforms, std::size_t count) const;
    void draw_depth_batch_instanced_impl(const std::string &model_name,
                                         const util::TransformData *transforms, std::size_t count) const;

    void initialize() override;
    void begin_draw() override;

    void apply_effect(const resources::Shader *shader, const glm::mat4 &transform, Effect effect, const LightSource *light_override = nullptr) const;

    LightingController *m_lighting = nullptr;
    ShadowController *m_shadow = nullptr;
    GraphicsController *m_graphics = nullptr;
    Camera *m_camera = nullptr;
    resources::ResourcesController *m_resources = nullptr;
    resources::Shader *m_depth_shader = nullptr;
    resources::Shader *m_depth_instanced_shader = nullptr;
    std::function<void()> m_depth_scene_callback;
};

}
