#pragma once
#include <glm/vec3.hpp>
#include <string>

static constexpr auto X_AXIS = glm::vec3(1, 0, 0);
static constexpr auto Y_AXIS = glm::vec3(0, 1, 0);
static constexpr auto Z_AXIS = glm::vec3(0, 0, 1);
static constexpr auto CENTER = glm::vec3(0, 0, 0);

static constexpr int WINDOW_WIDTH = 1400;
static constexpr int WINDOW_HEIGHT = 1000;
static constexpr std::string WINDOW_TITLE = "Starfall Shores";