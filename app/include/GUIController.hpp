#pragma once
#include <engine/core/Engine.hpp>

namespace app {
    class GUIController final : public engine::core::Controller {
    public:
        [[nodiscard]] std::string_view name() const override {
            return "test::app::GUIController";
        }

    private:
        void initialize() override;

        void poll_events() override;

        void draw() override;
    };
}
