#pragma once
#include <glm/vec3.hpp>

static constexpr auto X_AXIS = glm::vec3(1, 0, 0);
static constexpr auto Y_AXIS = glm::vec3(0, 1, 0);
static constexpr auto Z_AXIS = glm::vec3(0, 0, 1);
static constexpr auto CENTER = glm::vec3(0, 0, 0);
static constexpr float FAR_PLANE = 250.0f;

using vec3 = glm::vec3;
using mat4 = glm::mat4;