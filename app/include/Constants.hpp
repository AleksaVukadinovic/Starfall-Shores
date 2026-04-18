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

static constexpr uint32_t SHADOW_MAP_SIZE = 4096;
static constexpr float SHADOW_ORTHO_SIZE  = 120.0f;
static constexpr float SHADOW_NEAR        = 0.1f;
static constexpr float SHADOW_FAR         = 200.0f;