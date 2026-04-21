#include <engine/platform/TimeController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <cstdio>

namespace engine::platform {

void TimeController::initialize() {
    m_resources = get<resources::ResourcesController>();
    m_was_night = is_night();
}

void TimeController::update() {
    if (!m_running)
        return;

    const float dt = get<PlatformController>()->dt();
    m_in_game_hours += dt * m_tickrate;

    if (m_in_game_hours >= HOURS_IN_DAY)
        m_in_game_hours -= HOURS_IN_DAY;
    if (m_in_game_hours < 0.0f)
        m_in_game_hours += HOURS_IN_DAY;

    m_was_night = is_night();
}

bool TimeController::is_night() const {
    return m_in_game_hours >= NIGHT_START || m_in_game_hours < NIGHT_END;
}

std::string TimeController::formatted_time() const {
    char buf[6];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", hours(), minutes());
    return buf;
}

} // namespace engine::platform
