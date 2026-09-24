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

    if (walk_mode && ground_height) {
        walk(dt);
    } else {
        fly(dt);
    }

    if (platform->key(key_rotate_left).is_down())
        m_camera->rotate_camera(-keyboard_rotate_speed, 0);
    if (platform->key(key_rotate_right).is_down())
        m_camera->rotate_camera(keyboard_rotate_speed, 0);
    if (platform->key(key_toggle_cursor).state() == platform::Key::State::JustPressed)
        platform->set_enable_cursor(!platform->is_cursor_enabled());
}

void FPSCameraController::fly(const float dt) const {
    const auto platform = get<platform::PlatformController>();
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
}

glm::vec3 FPSCameraController::walk_direction() const {
    // Walking follows the view direction flattened onto the ground, so looking up or down doesn't change the speed.
    const auto platform = get<platform::PlatformController>();
    const glm::vec3 forward = glm::normalize(glm::vec3(m_camera->Front.x, 0.0f, m_camera->Front.z));
    const glm::vec3 right   = glm::normalize(glm::vec3(m_camera->Right.x, 0.0f, m_camera->Right.z));
    auto held = [&](const platform::KeyId primary, const platform::KeyId alt) {
        return platform->key(primary).is_down() || platform->key(alt).is_down() ? 1.0f : 0.0f;
    };
    const glm::vec3 direction = forward * (held(key_forward, key_forward_alt) - held(key_backward, key_backward_alt))
                              + right * (held(key_right, key_right_alt) - held(key_left, key_left_alt));
    return glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0.0f);
}

void FPSCameraController::walk(const float dt) {
    const auto platform = get<platform::PlatformController>();
    glm::vec3 &position = m_camera->Position;

    // Horizontal movement, unless it leads up a slope that's too steep or off the walkable ground.
    const float speed          = platform->key(key_sprint).is_down() ? run_speed : walk_speed;
    const glm::vec3 direction  = walk_direction();
    const glm::vec2 step       = glm::vec2(direction.x, direction.z) * speed * dt;
    const glm::vec2 here   = glm::vec2(position.x, position.z);
    const auto ground_here = ground_height(here);
    const auto ground_next = ground_height(here + step);
    const bool can_move    = ground_next ? !ground_here || *ground_next - *ground_here <= max_slope * glm::length(step)
                                         : !ground_here; // off the walkable ground already: move freely back onto it
    if (can_move) {
        position.x += step.x;
        position.z += step.y;
    }

    // Vertical movement: jumping, falling, and standing on the ground.
    const auto ground = ground_height(glm::vec2(position.x, position.z));
    if (!ground) {
        // Nothing to stand on below (e.g. walk mode was turned on above the sea): hover until back over the ground.
        m_vertical_speed = 0.0f;
        m_on_ground      = false;
        return;
    }
    if (m_on_ground && platform->key(key_up).is_down()) {
        m_vertical_speed = jump_speed;
        m_on_ground      = false;
    }
    m_vertical_speed -= gravity * dt;
    position.y += m_vertical_speed * dt;

    const float standing_height = *ground + eye_height;
    // While walking downhill, follow the ground down instead of stepping off it into a fall.
    const bool follows_ground = m_on_ground && m_vertical_speed <= 0.0f &&
                                position.y - standing_height <= max_slope * glm::length(step) + GROUND_SNAP_DISTANCE;
    m_on_ground = position.y <= standing_height || follows_ground;
    if (m_on_ground) {
        position.y       = standing_height;
        m_vertical_speed = 0.0f;
    }
}

}
