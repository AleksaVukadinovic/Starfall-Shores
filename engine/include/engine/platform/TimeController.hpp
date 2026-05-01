#pragma once

#include <engine/core/Controller.hpp>
#include <string>

namespace engine::resources {
class Skybox;
class ResourcesController;
}

namespace engine::platform {

class TimeController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "TimeController";
    }

    [[nodiscard]] float in_game_hours() const { return m_in_game_hours; }
    [[nodiscard]] int hours() const { return static_cast<int>(m_in_game_hours); }
    [[nodiscard]] int minutes() const { return static_cast<int>((m_in_game_hours - static_cast<float>(hours())) * 60.0f); }
    [[nodiscard]] std::string formatted_time() const;

    [[nodiscard]] bool is_night() const;
    [[nodiscard]] bool is_running() const { return m_running; }

    void set_running(const bool running) { m_running = running; }
    void set_tickrate(const float tickrate) { m_tickrate = tickrate; }
    [[nodiscard]] float tickrate() const { return m_tickrate; }

    static constexpr float NIGHT_START = 23.0f;
    static constexpr float NIGHT_END = 7.0f;
    static constexpr float HOURS_IN_DAY = 24.0f;

private:
    void initialize() override;
    void update() override;

    float m_in_game_hours = 12.0f;
    float m_tickrate = 0.2f;
    bool m_running = true;

    std::string m_day_skybox_name = "skybox_day";
    std::string m_night_skybox_name = "skybox_night";
    bool m_was_night = false;

    resources::ResourcesController *m_resources = nullptr;
};

} // namespace engine::platform
