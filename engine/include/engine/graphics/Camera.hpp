/**
 * @file Camera.hpp
 * @brief Defines the Camera class for rendering.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {
/**
 *  @class Camera
 *  @brief Camera processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL.
 */
class Camera {
public:
    /**
     * @brief  Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
     */
    enum Movement {
        None,
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    /**
     * @brief  Default camera values
     */

    static constexpr float YAW = -90.0f;
    static constexpr float PITCH = 0.0f;
    static constexpr float SPEED = 2.5f;
    static constexpr float SENSITIVITY = 0.1f;
    static constexpr float ZOOM = 45.0f;

    /**
    * @brief Defines a position in World-Space of where the Camera is located.
    */
    glm::vec3 Position{};

    /**
    * @brief Defines a vector in which the camera is pointed relative to the @ref Camera::Position.
    */
    glm::vec3 Front{};

    /**
    * @brief Defines the Up vector in the World-Space.
    */
    glm::vec3 Up{};

    /**
    * @brief Defines the camera Right vector.
    */
    glm::vec3 Right{};

    /**
    * @brief Defines the up orientation of the World-Space.
    */
    glm::vec3 WorldUp{};

    float Yaw{};
    float Pitch{};

    /**
    * @brief Used in @ref Camera::move_camera. The higher the value the faster the camera will move on keyboard action.
    */
    float MovementSpeed{};

    /**
    * @brief Used in @ref Camera::rotate_camera. The higher the value the faster the camera will move on mouse action.
    */
    float MouseSensitivity{};

    /**
    * @brief Defines camera FOV.
    */
    float Zoom{};

    /**
     * @brief  constructor with vectors.
     */
    explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yaw = YAW, float pitch = PITCH);

    /**
     * @brief  constructor with scalar values.
     */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    /**
     * @returns  returns the view matrix calculated using Euler Angles and the LookAt Matrix.
     */
    glm::mat4 view_matrix() const;

    /**
     * @brief Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems).
     */
    void move_camera(Movement direction, float deltaTime);

    /**
     * @brief Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
     */
    void rotate_camera(float x_offset, float y_offset, bool constrainPitch = true);

    /**
     * @brief Processes input received from a mouse scroll-wheel event. Only requires to be input on the vertical wheel-axis
     */
    void zoom(float offset);

private:
    /**
     * @brief Calculates the front vector from the Camera's (updated) Euler Angles
     */
    void update_camera_vectors();
};
}
#endif





