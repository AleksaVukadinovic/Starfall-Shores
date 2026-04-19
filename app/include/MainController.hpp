#pragma once
#include <array>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace engine::resources {
    class ResourcesController;
}

namespace app {
    struct PlacedModel {
        std::string model_name;
        glm::vec3 translation;
        glm::vec3 rotation;
        float scale;
    };

    using TreeData = std::array<float, 4>;
    struct OldTreeData { float x, y, z, scale, rotation_angle, rx, ry, rz; };

    template<typename Container, typename TransformFn>
    std::vector<glm::mat4> build_instanced_matrices(const Container &data, TransformFn transform_fn) {
        std::vector<glm::mat4> matrices;
        matrices.reserve(data.size());
        for (const auto &entry : data)
            matrices.push_back(transform_fn(entry));
        return matrices;
    }

    class MainController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::MainController";
        }

        void set_skybox(const std::string &new_skybox, bool is_daytime_skybox);

        static constexpr std::array TEST_MODEL_NAMES = {
            "terrain", "campfire", "log_seat", "viking_tent", "stylized_tent",
            "yellow_tree", "green_tree", "beech_tree", "pine_tree", "oak_tree", "old_tree",
            "bush1", "bush2", "laurel_bush", "flowers2", "red_flowers",
            "path", "shrooms", "grave", "fire", "water", "grass", "grass"
        };

        int selected_model_index = 0;
        glm::vec3 test_translation{0.0f, 0.0f, 0.0f};
        glm::vec3 test_rotation{0.0f, 0.0f, 0.0f};
        float test_scale = 1.0f;
        std::vector<PlacedModel> placed_models;

        void place_test_model();
        void clear_placed_models();

        bool wind_enabled = false;
        float wind_intensity = 0.8f;

    private:
        void initialize() override;
        bool loop() override;
        void draw() override;
        void draw_skybox() const;
        void draw_terrain() const;
        void draw_water() const;
        void draw_campfire() const;
        void draw_logs() const;
        void draw_tents() const;
        void draw_bushes() const;
        void draw_flowers() const;
        void draw_path() const;
        void draw_mushrooms() const;
        void draw_grave() const;
        void draw_grass() const;
        void draw_fire() const;
        void draw_test_model() const;

        void draw_trees(const engine::resources::Shader *shader) const;

        void render_depth_scene() const;

        void update() override;
        void terminate() override;
        void update_day_night_transition();
        void update_toggles();

        engine::resources::ResourcesController *m_resources = nullptr;
        engine::resources::Shader *m_basic_shader           = nullptr;
        engine::graphics::GraphicsController *m_graphics    = nullptr;
        engine::graphics::PostProcessingController *m_bloom = nullptr;
        engine::graphics::ShadowController *m_shadow        = nullptr;
        engine::graphics::LightingController *m_lighting    = nullptr;
        engine::graphics::Renderer *m_renderer              = nullptr;
        FogController *m_fog                                = nullptr;
        engine::graphics::Camera *m_camera                  = nullptr;

        bool m_is_day                            = true;
        bool m_day_change_requested              = false;
        double m_day_change_timer                = 0.0;
        float m_current_exposure                 = DAY_EXPOSURE;
        double m_fire_start_time                 = 0.0;
        static constexpr double DAY_CHANGE_DELAY = 5.0;
        static constexpr float DAY_EXPOSURE      = 1.3f;
        static constexpr float NIGHT_EXPOSURE    = 0.4f;
        std::string m_active_daytime_skybox      = "skybox_day";
        std::string m_active_nighttime_skybox    = "skybox_night";

        static constexpr auto WATER_COLOR_DAY = glm::vec3(0.0f, 0.4f, 0.6f);
        static constexpr auto WATER_COLOR_NIGHT = glm::vec3(0.0f, 0.1f, 0.3f);
        static constexpr auto LIGHT_POS_DAY = glm::vec3(0.0f, 60.0f, 0.0f);
        static constexpr auto LIGHT_POS_NIGHT = glm::vec3(12.0f, 25.0f, 6.0f);
        static constexpr auto LIGHT_COLOR_DAY = glm::vec3(1.0f, 1.0f, 1.0f);
        static constexpr auto LIGHT_COLOR_NIGHT = glm::vec3(1.0f, 0.7f, 0.1f);
        static constexpr auto AMBIENT_LIGHT_DAY = glm::vec3(0.2f);
        static constexpr auto AMBIENT_LIGHT_NIGHT = glm::vec3(0.05f);
        static constexpr auto DIFFUSE_LIGHT_DAY = glm::vec3(0.5f);
        static constexpr auto DIFFUSE_LIGHT_NIGHT = glm::vec3(0.2f);
        static constexpr auto SPECULAR_LIGHT_DAY = glm::vec3(0.1f);
        static constexpr auto SPECULAR_LIGHT_NIGHT = glm::vec3(0.05f);
        static constexpr auto SHININESS_DAY = 1024.0f;
        static constexpr auto SHININESS_NIGHT = 2048.0f;

        void apply_day_night_lighting() const;

        engine::resources::Shader *m_depth_shader           = nullptr;
        engine::resources::Shader *m_depth_instanced_shader = nullptr;
    };
}
