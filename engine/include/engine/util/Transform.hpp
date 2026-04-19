/**
 * @file Transform.hpp
 * @brief Utility functions for building model transformation matrices.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::util {

inline constexpr auto X_AXIS = glm::vec3(1, 0, 0);
inline constexpr auto Y_AXIS = glm::vec3(0, 1, 0);
inline constexpr auto Z_AXIS = glm::vec3(0, 0, 1);
inline constexpr auto ORIGIN = glm::vec3(0, 0, 0);

/**
 * @brief Creates a model matrix from per-axis Euler rotations (degrees), translation, and scale.
 *
 * Applies rotations in X → Y → Z order, then translation, then scale.
 *
 * @param translation Position offset.
 * @param rotation_degrees Euler angles in degrees (x, y, z).
 * @param scale Scale factor per axis.
 * @returns The composed model matrix.
 *
 * @code
 * auto m = model_matrix({10, 20, 5}, {-90, 0, -48}, glm::vec3(1.35f));
 * @endcode
 */
inline glm::mat4 model_matrix(const glm::vec3 &translation,
                              const glm::vec3 &rotation_degrees,
                              const glm::vec3 &scale) {
    auto m = glm::mat4(1.0f);
    m = glm::rotate(m, glm::radians(rotation_degrees.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(rotation_degrees.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(rotation_degrees.z), glm::vec3(0, 0, 1));
    m = glm::translate(m, translation);
    m = glm::scale(m, scale);
    return m;
}

/**
 * @brief Creates a model matrix from a single-axis rotation (degrees), translation, and uniform scale.
 *
 * @param translation Position offset.
 * @param uniform_scale Uniform scale factor applied to all axes.
 * @param rotation_axis The axis to rotate around.
 * @param rotation_angle_degrees Rotation angle in degrees.
 * @returns The composed model matrix.
 *
 * @code
 * auto m = model_matrix({10, 17, -14}, 0.037f, Y_AXIS, -20.0f);
 * @endcode
 */
inline glm::mat4 model_matrix(const glm::vec3 &translation,
                              const float uniform_scale,
                              const glm::vec3 &rotation_axis,
                              const float rotation_angle_degrees) {
    auto m = glm::mat4(1.0f);
    m = glm::rotate(m, glm::radians(rotation_angle_degrees), rotation_axis);
    m = glm::translate(m, translation);
    m = glm::scale(m, glm::vec3(uniform_scale));
    return m;
}

/**
 * @brief Creates a model matrix from a single-axis rotation (degrees), translation, and non-uniform scale.
 *
 * @param translation Position offset.
 * @param scale Scale factor per axis.
 * @param rotation_axis The axis to rotate around.
 * @param rotation_angle_degrees Rotation angle in degrees.
 * @returns The composed model matrix.
 */
inline glm::mat4 model_matrix(const glm::vec3 &translation,
                              const glm::vec3 &scale,
                              const glm::vec3 &rotation_axis,
                              const float rotation_angle_degrees) {
    auto m = glm::mat4(1.0f);
    m = glm::rotate(m, glm::radians(rotation_angle_degrees), rotation_axis);
    m = glm::translate(m, translation);
    m = glm::scale(m, scale);
    return m;
}

/**
 * @brief Creates a translation-only model matrix.
 *
 * @param translation Position offset.
 * @returns The composed model matrix.
 *
 * @code
 * auto m = model_matrix({12.0f, 17.3f, 6.0f});
 * @endcode
 */
inline glm::mat4 model_matrix(const glm::vec3 &translation) {
    return glm::translate(glm::mat4(1.0f), translation);
}

}// namespace engine::util
