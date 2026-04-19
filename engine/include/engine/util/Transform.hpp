/**
 * @file Transform.hpp
 * @brief Utility functions for building model transformation matrices.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <future>

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
 */
glm::mat4 model_matrix(const glm::vec3 &translation,
                       const glm::vec3 &rotation_degrees,
                       const glm::vec3 &scale);

/**
 * @brief Creates a model matrix from a single-axis rotation (degrees), translation, and uniform scale.
 *
 * @param translation Position offset.
 * @param uniform_scale Uniform scale factor applied to all axes.
 * @param rotation_axis The axis to rotate around.
 * @param rotation_angle_degrees Rotation angle in degrees.
 * @returns The composed model matrix.
 */
glm::mat4 model_matrix(const glm::vec3 &translation,
                       float uniform_scale,
                       const glm::vec3 &rotation_axis,
                       float rotation_angle_degrees);

/**
 * @brief Creates a model matrix from a single-axis rotation (degrees), translation, and non-uniform scale.
 *
 * @param translation Position offset.
 * @param scale Scale factor per axis.
 * @param rotation_axis The axis to rotate around.
 * @param rotation_angle_degrees Rotation angle in degrees.
 * @returns The composed model matrix.
 */
glm::mat4 model_matrix(const glm::vec3 &translation,
                       const glm::vec3 &scale,
                       const glm::vec3 &rotation_axis,
                       float rotation_angle_degrees);

/**
 * @brief Creates a translation-only model matrix.
 *
 * @param translation Position offset.
 * @returns The composed model matrix.
 */
glm::mat4 model_matrix(const glm::vec3 &translation);

/**
 * @brief Builds a vector of model matrices by applying a transform function to each element.
 *
 * @param data Container of source data.
 * @param transform_fn Function mapping each element to a glm::mat4.
 * @returns Vector of model matrices.
 */
template<typename Container, typename TransformFn>
std::vector<glm::mat4> build_instance_matrices(const Container &data, TransformFn transform_fn) {
    std::vector<glm::mat4> matrices;
    matrices.reserve(data.size());
    for (const auto &entry : data)
        matrices.push_back(transform_fn(entry));
    return matrices;
}

/**
 * @brief Asynchronously builds a vector of model matrices.
 *
 * Launches the matrix building on a separate thread and returns a future.
 *
 * @param data Container of source data.
 * @param transform_fn Function mapping each element to a glm::mat4.
 * @returns A future that resolves to the vector of model matrices.
 */
template<typename Container, typename TransformFn>
std::future<std::vector<glm::mat4>> build_instance_matrices_async(const Container &data, TransformFn transform_fn) {
    return std::async(std::launch::async, [&data, transform_fn = std::move(transform_fn)] {
        return build_instance_matrices(data, transform_fn);
    });
}

}// namespace engine::util
