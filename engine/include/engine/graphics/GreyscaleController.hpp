#pragma once

#include <engine/core/Controller.hpp>

class GreyscaleController : public engine::core::Controller {
public:
    bool greyscale_enabled   = false;
    float greyscale_strength = 1.0f;

    [[nodiscard]] std::string_view name() const override {
        return "GreyscaleController";
    }
};
