#pragma once
#include <MainController.hpp>
#include "engine/graphics/GraphicsController.hpp"
#include <engine/core/Engine.hpp>
#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/RainController.hpp>

namespace app {
    class GUIController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::GUIController";
        }

    private:
        engine::graphics::GraphicsController *m_graphics = nullptr;
        engine::graphics::Camera *m_camera = nullptr;
        engine::graphics::PostProcessingController *m_bloom = nullptr;
        engine::graphics::FPSCameraController *m_fps_camera = nullptr;
        engine::platform::PlatformController *m_platform = nullptr;
        FogController *m_fog = nullptr;
        GreyscaleController *m_greyscale = nullptr;
        RainController *m_rain = nullptr;
        MainController* m_main_controller = nullptr;

        void initialize() override;
        void poll_events() override;
        void draw() override;

        void draw_bloom_controls() const;
        void draw_fog_controls() const;
        void draw_greyscale_controls() const;
        void draw_rain_controls() const;
        void draw_wind_controls() const;
        void draw_shadow_controls() const;
        void draw_skybox_controls() const;
        void draw_test_controls() const;
        void draw_camera_info() const;
        void draw_fps_counter();
        void draw_tickrate_slider();
        void draw_fov_slider();

        float m_fps_accumulator = 0.0f;
        int m_fps_frame_count = 0;
        float m_last_update_time = 0.0f;
        float m_last_recorded_fps = 0.0f;

        float m_tickrate = 1.0f;
        float m_fov = 0.0f;
    };
}
