#pragma once
#include <engine/platform/TimeController.hpp>
#include <array>
#include <optional>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/Scene.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/graphics/WaterController.hpp>
#include <engine/util/Transform.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace engine::resources {
    class Model;
    class ResourcesController;
    class Shader;
    class Sound;
}

namespace app {
    struct PlacedModel {
        std::string model_name;
        const engine::resources::Model *model;
        glm::vec3 translation;
        glm::vec3 rotation;
        float scale;
    };


    class MainController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::MainController";
        }

        void set_skybox(const std::string &new_skybox, bool is_daytime_skybox);

        static constexpr std::array TEST_MODEL_NAMES = {
            "terrain", "campfire", "log_seat", "viking_tent", "stylized_tent",
            "yellow_tree", "green_tree", "beech_tree", "pine_tree", "oak_tree", "old_tree",
            "bush1", "bush2", "laurel_bush", "white_flowers", "red_flowers",
            "path", "shrooms", "grave", "fire", "grass", "lighthouse", "crow", "seagull"
        };

        /**
         * @brief Set by the GUI while the model placement tool is open; the model being placed is only drawn then.
         */
        bool model_placement_active = false;
        int selected_model_index = 0;
        glm::vec3 test_translation{0.0f, 0.0f, 0.0f};
        glm::vec3 test_rotation{0.0f, 0.0f, 0.0f};
        float test_scale = 1.0f;
        std::vector<PlacedModel> placed_models;
        bool m_sound_enabled = false;
        float m_sound_volume = 80.0f;

        void place_test_model();
        void clear_placed_models();

    private:
        void initialize() override;
        bool loop() override;
        void draw() override;

        void create_scene();
        void draw_skybox() const;
        void draw_placed_models() const;

        void render_depth_scene() const;

        /**
         * @brief Height of the solid ground: the scene colliders (the terrain), or the sea floor where it's higher.
         */
        [[nodiscard]] std::optional<float> solid_ground(glm::vec2 xz) const;

        /**
         * @brief Switches the underwater-only effects (particles, light shafts, muffled sound) as the camera dives or surfaces.
         */
        void update_underwater();

        void update() override;
        void terminate() override;
        void update_sound() const;
        void update_day_night_transition();
        void update_toggles();

        engine::resources::ResourcesController *m_resources = nullptr;
        engine::graphics::GraphicsController *m_graphics    = nullptr;
        engine::graphics::PostProcessingController *m_bloom = nullptr;
        engine::graphics::ShadowController *m_shadow        = nullptr;
        engine::graphics::LightingController *m_lighting    = nullptr;
        engine::graphics::Renderer *m_renderer              = nullptr;
        engine::platform::TimeController *m_time = nullptr;
        FogController *m_fog                                = nullptr;
        engine::graphics::WaterController *m_water          = nullptr;
        engine::graphics::Camera *m_camera                  = nullptr;
        engine::resources::Sound *m_background_sound               = nullptr;
        const engine::resources::Shader *m_placement_shader = nullptr;

        engine::graphics::Scene m_scene;
        static constexpr std::string_view SCENE_PATH = "resources/scene.json";
        /**
         * @brief Scene entities and particles in these groups are only shown at night (like the campfire flames)
         * or during the day (like pollen).
         */
        static constexpr std::string_view NIGHT_GROUP = "night";
        static constexpr std::string_view DAY_GROUP = "day";
        /**
         * @brief Scene particles in this group (plankton, bubbles) are only shown while the camera is underwater.
         */
        static constexpr std::string_view UNDERWATER_GROUP = "underwater";
        static constexpr float MUFFLING_FADE_SECONDS = 0.3f;
        bool m_was_underwater = false;
        float m_muffling = 0.0f;

        bool m_is_day                            = true;
        bool m_day_change_requested              = false;
        double m_day_change_timer                = 0.0;
        float m_current_exposure                 = DAY_EXPOSURE;
        static constexpr double DAY_CHANGE_DELAY = 5.0;
        static constexpr float DAY_EXPOSURE      = 1.3f;
        static constexpr float NIGHT_EXPOSURE    = 0.4f;
        std::string m_active_daytime_skybox      = "skybox_day";
        std::string m_active_nighttime_skybox    = "skybox_night";

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
    };
}
