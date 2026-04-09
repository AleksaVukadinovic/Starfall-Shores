#pragma once
#include <spdlog/spdlog.h>
#include <engine/graphics/GraphicsController.hpp>

namespace app {
    class MainController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::MainController";
        }

        void set_skybox(const std::string &new_skybox, bool is_daytime_skybox);

    private:
        void initialize() override;
        bool loop() override;
        void begin_draw() override;
        void end_draw() override;
        void draw() override;
        void draw_skybox() const;
        void draw_terrain() const;
        void draw_water() const;
        void draw_forest() const;
        void draw_campfire() const;
        void draw_logs() const;
        void draw_tents() const;
        void draw_bushes() const;
        void draw_white_flowers() const;
        void draw_red_flowers() const;
        void draw_flowers() const;
        void draw_path() const;
        void draw_mushrooms() const;

        void draw_stones() const;

        static void draw_fire();

        void update() override;

        void update_day_night_transition();

        static void update_camera();

        bool m_is_day                            = true;
        bool m_day_change_requested              = false;
        double m_day_change_timer                = 0.0;
        float m_current_exposure                 = DAY_EXPOSURE;
        static constexpr double DAY_CHANGE_DELAY = 5.0;
        static constexpr float DAY_EXPOSURE      = 1.3f;
        static constexpr float NIGHT_EXPOSURE    = 0.4f;
        std::string m_active_daytime_skybox        = "skybox_day";
        std::string m_active_nighttime_skybox      = "skybox_night";

        const glm::vec3 LIGHT_POS_DAY = glm::vec3(0.0f, 60.0f, 0.0f);
        const glm::vec3 LIGHT_POS_NIGHT = glm::vec3(12.0f, 25.0f, 6.0f);
        const glm::vec3 WATER_COLOR_DAY = glm::vec3(0.0f, 0.4f, 0.6f);
        const glm::vec3 WATER_COLOR_NIGHT = glm::vec3(0.0f, 0.1f, 0.3f);

        void set_common_shader_variables(const engine::resources::Shader *shader,
                                         const engine::graphics::Camera *camera,
                                         const engine::graphics::GraphicsController *graphics) const;
    };
}
