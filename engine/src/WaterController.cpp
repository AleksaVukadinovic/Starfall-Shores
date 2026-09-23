#include <glad/glad.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/WaterController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Skybox.hpp>
#include <engine/util/Errors.hpp>
#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <vector>

namespace engine::graphics {

/**
 * @brief Each wave is shorter than the dominant one by these ratios, and travels at these angles off the wind direction.
 * Amplitudes scale with length, so all the waves are equally steep.
 */
static constexpr std::array<float, WaterController::WAVE_COUNT> WAVE_LENGTH_RATIOS = {1.0f, 0.62f, 0.41f, 0.27f, 0.18f, 0.11f};
static constexpr std::array<float, WaterController::WAVE_COUNT> WAVE_DIRECTION_OFFSETS = {0.0f, 25.0f, -20.0f, 45.0f, -40.0f, 70.0f};

void WaterController::initialize() {
    m_shader = get<resources::ResourcesController>()->shader("water");
    m_shader->use();
    m_shader->set_int("environmentMap", ENVIRONMENT_TEXTURE_UNIT);
    create_grid();
}

void WaterController::terminate() {
    CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
    CHECKED_GL_CALL(glDeleteBuffers, 1, &m_vbo);
    CHECKED_GL_CALL(glDeleteBuffers, 1, &m_ebo);
}

float WaterController::grid_cell_size() {
    return GRID_EXTENT * GRID_LINEAR_SHARE * 2.0f / static_cast<float>(GRID_RESOLUTION);
}

void WaterController::create_grid() {
    // Grid lines are spaced by a blend of a linear and a cubic curve: fine around the camera, coarse at the horizon.
    auto grid_line = [](const uint32_t index) {
        const float u = static_cast<float>(index) / static_cast<float>(GRID_RESOLUTION) * 2.0f - 1.0f;
        return GRID_EXTENT * (GRID_LINEAR_SHARE * u + (1.0f - GRID_LINEAR_SHARE) * u * u * u);
    };
    constexpr uint32_t side = GRID_RESOLUTION + 1;
    std::vector<glm::vec2> vertices;
    vertices.reserve(side * side);
    for (uint32_t z = 0; z < side; ++z) {
        for (uint32_t x = 0; x < side; ++x) {
            vertices.emplace_back(grid_line(x), grid_line(z));
        }
    }
    std::vector<uint32_t> indices;
    indices.reserve(GRID_RESOLUTION * GRID_RESOLUTION * 6);
    for (uint32_t z = 0; z < GRID_RESOLUTION; ++z) {
        for (uint32_t x = 0; x < GRID_RESOLUTION; ++x) {
            const uint32_t corner = z * side + x;
            indices.insert(indices.end(), {corner, corner + side, corner + 1, corner + 1, corner + side, corner + side + 1});
        }
    }
    m_index_count = static_cast<uint32_t>(indices.size());

    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_vbo);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_ebo);
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, static_cast<int64_t>(vertices.size() * sizeof(glm::vec2)), vertices.data(), GL_STATIC_DRAW);
    CHECKED_GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    CHECKED_GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, static_cast<int64_t>(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, static_cast<int32_t>(sizeof(glm::vec2)), nullptr);
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

WaterController::Waves WaterController::waves() const {
    Waves result;
    for (std::size_t i = 0; i < WAVE_COUNT; ++i) {
        const float angle  = glm::radians(wind_direction + WAVE_DIRECTION_OFFSETS[i]);
        const float length = wavelength * WAVE_LENGTH_RATIOS[i];
        result[i] = glm::vec4(std::cos(angle), std::sin(angle), glm::two_pi<float>() / length, wave_height * WAVE_LENGTH_RATIOS[i]);
    }
    return result;
}

void WaterController::draw() {
    RG_GUARANTEE(reflected_skybox != nullptr, "Set WaterController::reflected_skybox before the water is drawn.");
    const auto graphics = get<GraphicsController>();
    graphics->profiler().begin_section("Water");

    // Move the grid in whole cells, so the vertices near the camera always sample the waves at the same places.
    const glm::vec3 &camera = graphics->camera()->Position;
    const float cell        = grid_cell_size();
    const glm::vec2 origin  = glm::floor(glm::vec2(camera.x, camera.z) / cell) * cell;

    const Waves wave_data = waves();
    float total_amplitude = 0.0f, total_slope = 0.0f;
    for (const auto &wave: wave_data) {
        total_amplitude += wave.w;
        total_slope += wave.z * wave.w;
    }
    // Crests would fold over themselves (loop) if the horizontal displacement outgrew the wave slopes.
    const float max_choppiness = total_slope > 0.0f ? 1.0f / total_slope : 0.0f;
    const float wind_angle     = glm::radians(wind_direction);

    m_shader->use();
    m_shader->set_vec2("gridOrigin", origin);
    m_shader->set_float("waterLevel", level);
    m_shader->set_vec4_array("waves", wave_data);
    m_shader->set_float("choppiness", std::min(choppiness, max_choppiness));
    m_shader->set_float("waveSpeed", wave_speed);
    m_shader->set_float("totalAmplitude", total_amplitude);
    m_shader->set_vec2("windDirection", glm::vec2(std::cos(wind_angle), std::sin(wind_angle)));
    m_shader->set_vec3("deepColor", deep_color);
    m_shader->set_vec3("shallowColor", shallow_color);
    m_shader->set_float("foamAmount", foam);
    m_shader->set_float("opacity", opacity);

    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + ENVIRONMENT_TEXTURE_UNIT);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, reflected_skybox->texture());
    OpenGL::enable_blending();
    CHECKED_GL_CALL(glBindVertexArray, m_vao);
    CHECKED_GL_CALL(glDrawElements, GL_TRIANGLES, static_cast<int32_t>(m_index_count), GL_UNSIGNED_INT, nullptr);
    OpenGL::disable_blending();
}

}
