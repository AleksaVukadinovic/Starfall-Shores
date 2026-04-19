#pragma once
#include <engine/util/Transform.hpp>
#include <string>

using engine::util::X_AXIS;
using engine::util::Y_AXIS;
using engine::util::Z_AXIS;
using engine::util::ORIGIN;

static constexpr int WINDOW_WIDTH = 1400;
static constexpr int WINDOW_HEIGHT = 1000;
static constexpr std::string WINDOW_TITLE = "Starfall Shores";

static constexpr uint32_t SHADOW_MAP_SIZE = 4096;
static constexpr float SHADOW_ORTHO_SIZE  = 120.0f;
static constexpr float SHADOW_NEAR        = 0.1f;
static constexpr float SHADOW_FAR         = 200.0f;