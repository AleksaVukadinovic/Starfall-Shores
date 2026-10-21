#include <engine/graphics/FlockController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/util/Transform.hpp>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {

void FlockController::add_flock(const FlockSettings &settings, const resources::Model *model, const resources::Shader *shader) {
    Flock flock{settings, model, shader, body_transform(settings, *model), {}, {}};
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    flock.members.reserve(settings.count);
    for (uint32_t i = 0; i < settings.count; ++i) {
        const glm::vec3 offset(unit(m_random), unit(m_random), unit(m_random));
        const glm::vec3 heading(unit(m_random), 0.0f, unit(m_random));
        flock.members.push_back({settings.center + offset * settings.extent * 0.5f,
                                 glm::normalize(heading + glm::vec3(0.001f)) * settings.min_speed});
    }
    flock.headings.resize(settings.count);
    m_flocks.push_back(std::move(flock));
}

glm::mat4 FlockController::body_transform(const FlockSettings &settings, const resources::Model &model) {
    // Turn the model to face +Z, move its center to the origin, and scale it to the flock's member length.
    const glm::mat4 rotation = util::model_matrix(glm::vec3(0.0f), settings.model_rotation, glm::vec3(1.0f));
    const util::AABB bounds  = model.bounds().transformed(rotation);
    const float model_length = std::max(bounds.max.z - bounds.min.z, 1e-4f);
    const float scale        = settings.length / model_length;
    return glm::scale(glm::mat4(1.0f), glm::vec3(scale)) * glm::translate(glm::mat4(1.0f), -bounds.center()) * rotation;
}

void FlockController::update() {
    const float dt = std::min(get<platform::PlatformController>()->dt(), MAX_TIME_STEP);
    for (auto &flock: m_flocks) {
        simulate(flock, dt);
    }
}

glm::vec3 FlockController::steering(const Flock &flock, const std::size_t member) {
    const FlockSettings &settings = flock.settings;
    const Member &self            = flock.members[member];
    glm::vec3 separation(0.0f), heading(0.0f), center(0.0f);
    int neighbours = 0;
    for (std::size_t other = 0; other < flock.members.size(); ++other) {
        const glm::vec3 offset = flock.members[other].position - self.position;
        const float distance2  = glm::dot(offset, offset);
        if (other == member || distance2 > settings.neighbour_radius * settings.neighbour_radius) {
            continue;
        }
        heading += flock.members[other].velocity;
        center += flock.members[other].position;
        ++neighbours;
        if (distance2 < settings.separation_distance * settings.separation_distance && distance2 > 1e-6f) {
            separation -= offset / distance2;
        }
    }

    glm::vec3 steer = separation * SEPARATION_WEIGHT;
    if (neighbours > 0) {
        steer += (heading / static_cast<float>(neighbours) - self.velocity) * ALIGNMENT_WEIGHT;
        steer += (center / static_cast<float>(neighbours) - self.position) * COHESION_WEIGHT;
    }
    // Turn back towards the center when leaving the box.
    const glm::vec3 relative = (self.position - settings.center) / settings.extent;
    steer -= glm::sign(relative) * glm::max(glm::abs(relative) - BOUNDS_MARGIN, glm::vec3(0.0f)) * BOUNDS_WEIGHT * settings.extent;
    // Rise above the ground.
    if (ground_height) {
        if (const auto ground = ground_height(glm::vec2(self.position.x, self.position.z));
            ground && self.position.y < *ground + settings.ground_clearance) {
            steer.y += (*ground + settings.ground_clearance - self.position.y) * GROUND_WEIGHT;
        }
    }
    // A little randomness, so the flock doesn't settle into a perfectly steady formation.
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    steer += glm::vec3(unit(m_random), unit(m_random) * 0.3f, unit(m_random)) * WANDER_WEIGHT;
    return steer;
}

void FlockController::simulate(Flock &flock, const float dt) {
    const FlockSettings &settings = flock.settings;
    for (std::size_t i = 0; i < flock.members.size(); ++i) {
        Member &member = flock.members[i];
        member.velocity += steering(flock, i) * dt;
        const float speed = glm::length(member.velocity);
        member.velocity *= std::clamp(speed, settings.min_speed, settings.max_speed) / std::max(speed, 1e-4f);
        member.position += member.velocity * dt;
        flock.headings[i] = heading(member);
    }
}

glm::mat4 FlockController::heading(const Member &member) {
    // Point the body's +Z along the velocity, keeping it upright.
    const glm::vec3 forward = glm::normalize(member.velocity);
    glm::vec3 right         = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward);
    right                   = glm::length(right) > 1e-4f ? glm::normalize(right) : glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 up      = glm::cross(forward, right);
    return {glm::vec4(right, 0.0f), glm::vec4(up, 0.0f), glm::vec4(forward, 0.0f), glm::vec4(member.position, 1.0f)};
}

void FlockController::draw() {
    const auto renderer = get<Renderer>();
    for (const auto &flock: m_flocks) {
        // The shader applies the body transform before the swimming motion, then each member's heading.
        flock.shader->use();
        flock.shader->set_mat4("bodyTransform", flock.body_transform);
        flock.shader->set_float("bodyLength", flock.settings.length);
        renderer->draw_instances(flock.model, flock.shader, flock.headings);
    }
}

}
