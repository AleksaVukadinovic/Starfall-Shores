#pragma once
#include <MainController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/platform/TimeController.hpp>

namespace app {

    enum class MenuPage {
        Main,
        Settings,
    };

    enum class SettingsCategory {
        Graphics,
        Environment,
        Sound,
        Time,
        Debug,
    };

    class GUIController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "app::GUIController";
        }

        bool m_wants_quit = false;
        bool m_menu_open = false;

    private:
        engine::graphics::GraphicsController *m_graphics = nullptr;
        engine::graphics::Camera *m_camera = nullptr;
        engine::graphics::PostProcessingController *m_bloom = nullptr;
        engine::graphics::FPSCameraController *m_fps_camera = nullptr;
        engine::graphics::ShadowController *m_shadow = nullptr;
        engine::platform::PlatformController *m_platform = nullptr;
        FogController *m_fog = nullptr;
        GreyscaleController *m_greyscale = nullptr;
        RainController *m_rain = nullptr;
        MainController *m_main_controller = nullptr;
        engine::platform::TimeController *m_time = nullptr;

        MenuPage m_page = MenuPage::Main;
        SettingsCategory m_settings_category = SettingsCategory::Graphics;

        void initialize() override;
        void poll_events() override;
        void draw() override;

        static void push_style();
        static void pop_style();

        void draw_main_menu();
        void draw_settings();
        void draw_settings_sidebar();
        void draw_settings_content();

        void draw_graphics_settings();
        void draw_environment_settings() const;
        void draw_sound_settings() const;
        void draw_time_settings() const;
        void draw_debug_settings() const;

        float m_fps_accumulator = 0.0f;
        int m_fps_frame_count = 0;
        float m_last_recorded_fps = 0.0f;
        float m_fov = 0.0f;
        int m_resolution_index = 0;
        bool m_fullscreen = false;
    };
}
