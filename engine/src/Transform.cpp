#include <engine/util/Transform.hpp>

namespace engine::util {

glm::mat4 model_matrix(const glm::vec3 &translation,
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

glm::mat4 model_matrix(const glm::vec3 &translation,
                       const float uniform_scale,
                       const glm::vec3 &rotation_axis,
                       const float rotation_angle_degrees) {
    auto m = glm::mat4(1.0f);
    m = glm::rotate(m, glm::radians(rotation_angle_degrees), rotation_axis);
    m = glm::translate(m, translation);
    m = glm::scale(m, glm::vec3(uniform_scale));
    return m;
}

glm::mat4 model_matrix(const glm::vec3 &translation,
                       const glm::vec3 &scale,
                       const glm::vec3 &rotation_axis,
                       const float rotation_angle_degrees) {
    auto m = glm::mat4(1.0f);
    m = glm::rotate(m, glm::radians(rotation_angle_degrees), rotation_axis);
    m = glm::translate(m, translation);
    m = glm::scale(m, scale);
    return m;
}

glm::mat4 model_matrix(const glm::vec3 &translation) {
    return glm::translate(glm::mat4(1.0f), translation);
}

}
