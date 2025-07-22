
#ifndef GUICONTROLLER_HPP
#define GUICONTROLLER_HPP

#include <engine/core/Engine.hpp>

namespace engine::test::app {
class GUIController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "test::app::GUIController";
    }

private:
    void initialize() override;

    void poll_events() override;

    void draw() override;
};
}
#endif //GUICONTROLLER_HPP
