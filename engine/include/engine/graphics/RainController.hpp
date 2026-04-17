#pragma once

#include <engine/core/Controller.hpp>

class RainController : public engine::core::Controller {
public:
    bool rain_enabled       = false;
    float rain_intensity    = 0.5f;
    float rain_speed        = 1.0f;
    float rain_opacity      = 0.3f;
    float rain_streak_length = 0.15f;

    [[nodiscard]] std::string_view name() const override {
        return "RainController";
    }
};
