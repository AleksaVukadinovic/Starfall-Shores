#pragma once
#include "MainController.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include <engine/core/Engine.hpp>

namespace app {
    class GUIController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "test::app::GUIController";
        }

    private:
        engine::graphics::GraphicsController *m_graphics = nullptr;
        engine::graphics::Camera *m_camera = nullptr;
        engine::graphics::BloomController *m_bloom = nullptr;
        engine::platform::PlatformController *m_platform = nullptr;
        MainController* m_main_controller = nullptr;

        void initialize() override;
        void poll_events() override;
        void draw() override;

        void draw_bloom_controls() const;
        void draw_skybox_controls() const;
        void draw_camera_info() const;
        void draw_fps_counter() const;
    };
}
