
#include <engine/graphics/Camera.hpp>

namespace engine::graphics {
// constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    update_camera_vectors();
}

// constructor with scalar values
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    update_camera_vectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::view_matrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::move_camera(Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD) {
        Position += Front * velocity;
    }
    if (direction == BACKWARD) {
        Position -= Front * velocity;
    }
    if (direction == LEFT) {
        Position -= Right * velocity;
    }
    if (direction == RIGHT) {
        Position += Right * velocity;
    }
    if (direction == UP) {
        Position += Up * velocity;
    }
    if (direction == DOWN) {
        Position -= Up * velocity;
    }
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::rotate_camera(float x_offset, float y_offset, bool constrainPitch) {
    x_offset *= MouseSensitivity;
    y_offset *= MouseSensitivity;

    Yaw += x_offset;
    Pitch += y_offset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f) {
            Pitch = 89.0f;
        }
        if (Pitch < -89.0f) {
            Pitch = -89.0f;
        }

    }

    // update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors();
}

// processes input received from a mouse scroll-wheel event. Only requires to be input on the vertical wheel-axis
void Camera::zoom(float offset) {
    Zoom -= (float) offset;
    if (Zoom < 1.0f) {
        Zoom = 1.0f;
    }
    if (Zoom > 45.0f) {
        Zoom = 45.0f;
    }
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::update_camera_vectors() {
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cosf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));
    front.y = sinf(glm::radians(Pitch));
    front.z = sinf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}
}
