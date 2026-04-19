#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformController.hpp>

namespace engine::graphics {

class FPSCameraMouseObserver final : public platform::PlatformEventObserver {
public:
    explicit FPSCameraMouseObserver(FPSCameraController *controller, Camera *camera)
        : m_controller(controller), m_camera(camera) {}

    void on_mouse_move(const platform::MousePosition position) override {
        if (!m_controller->is_enabled())
            return;
        if (const auto platform = core::Controller::get<platform::PlatformController>(); platform->is_cursor_enabled())
            return;
        m_camera->rotate_camera(position.dx, position.dy);
    }

private:
    FPSCameraController *m_controller;
    Camera *m_camera;
};

void FPSCameraController::initialize() {
    m_camera = get<GraphicsController>()->camera();
    const auto platform = get<platform::PlatformController>();
    platform->register_platform_event_observer(
        std::make_unique<FPSCameraMouseObserver>(this, m_camera));
}

void FPSCameraController::update() {
    const auto platform = get<platform::PlatformController>();
    const float dt = platform->dt();

    m_camera->MovementSpeed = platform->key(key_sprint).is_down() ? sprint_speed : normal_speed;

    auto move = [&](const platform::KeyId primary, const platform::KeyId alt, const Camera::Movement dir) {
        if (platform->key(primary).is_down() || platform->key(alt).is_down())
            m_camera->move_camera(dir, dt);
    };

    move(key_forward, key_forward_alt, Camera::Movement::FORWARD);
    move(key_backward, key_backward_alt, Camera::Movement::BACKWARD);
    move(key_left, key_left_alt, Camera::Movement::LEFT);
    move(key_right, key_right_alt, Camera::Movement::RIGHT);

    if (platform->key(key_up).is_down())
        m_camera->move_camera(Camera::Movement::UP, dt);
    if (platform->key(key_down).is_down())
        m_camera->move_camera(Camera::Movement::DOWN, dt);
    if (platform->key(key_rotate_left).is_down())
        m_camera->rotate_camera(-keyboard_rotate_speed, 0);
    if (platform->key(key_rotate_right).is_down())
        m_camera->rotate_camera(keyboard_rotate_speed, 0);
    if (platform->key(key_toggle_cursor).state() == platform::Key::State::JustPressed)
        platform->set_enable_cursor(!platform->is_cursor_enabled());
}

}
