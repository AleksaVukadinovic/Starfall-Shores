#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP
#include <spdlog/spdlog.h>
#include <engine/graphics/GraphicsController.hpp>

namespace app {
    class MainController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::MainController";
        }

        void set_skybox(const std::string &new_skybox, const bool is_daytime_skybox) {
            if (m_is_day && is_daytime_skybox) {
                active_daytime_skybox = new_skybox;
            } else if (!m_is_day && !is_daytime_skybox) {
                active_nighttime_skybox = new_skybox;
            }
        }

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

        void draw_old_tree() const;

        void draw_bushes() const;

        void draw_white_flowers() const;

        void draw_red_flowers() const;

        void draw_flowers() const;

        void draw_path() const;

        void draw_mushrooms() const;

        void draw_stones() const;

        static void draw_fire();

        void update() override;

        void update_camera();

        bool m_is_day = true;
        std::string active_daytime_skybox = "skybox_day";
        std::string active_nighttime_skybox = "skybox_night";

        void set_common_shader_variables(const engine::resources::Shader *shader,
                                         const engine::graphics::Camera *camera,
                                         const engine::graphics::GraphicsController *graphics) const {
            const auto light_position = m_is_day ? glm::vec3(0.0f, 60.0f, 0.0f) : glm::vec3(12.0f, 25.0f, 6.0f);
            const auto ambient = m_is_day ? glm::vec3(0.2f) : glm::vec3(0.1f);
            const auto diffuse = m_is_day ? glm::vec3(0.5f) : glm::vec3(0.3f);
            const auto specular = m_is_day ? glm::vec3(0.1) : glm::vec3(0.05f);
            const float shininess = m_is_day ? 1024.0f : 2048.0f;
            const auto light_color = m_is_day ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(1.0f, 0.7f, 0.1f);
            shader->use();
            shader->set_vec3("light.position", light_position);
            shader->set_vec3("light.ambient", ambient);
            shader->set_vec3("light.diffuse", diffuse);
            shader->set_vec3("light.specular", specular);
            shader->set_vec3("lightColor", light_color);
            shader->set_float("material.shininess", shininess);
            shader->set_vec3("viewPos", camera->Position);
            shader->set_mat4("projection", graphics->projection_matrix());
            shader->set_mat4("view", camera->view_matrix());
        }
    };
}

#endif //MAINCONTROLLER_HPP