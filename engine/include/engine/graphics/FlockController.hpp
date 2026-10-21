/**
 * @file FlockController.hpp
 * @brief Schools of fish (or flocks of birds) that move together, simulated as boids.
 */

#pragma once

#include <engine/core/Controller.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace engine::resources {
class Model;
class Shader;
}

namespace engine::graphics {

/**
 * @brief Where a flock lives and how its members move.
 */
struct FlockSettings {
    std::string name;
    uint32_t count = 30;
    /**
     * @brief The flock stays inside a box around `center`, `extent` from it on each axis.
     */
    glm::vec3 center{0.0f};
    glm::vec3 extent{10.0f};
    /**
     * @brief Length of one member in world units; the model is scaled to it.
     */
    float length = 1.0f;
    /**
     * @brief Rotation in degrees around x, y, z that turns the model to face +Z with +Y up, for models authored facing another way.
     */
    glm::vec3 model_rotation{0.0f};
    float min_speed = 1.0f;
    float max_speed = 3.0f;
    /**
     * @brief Members match the heading of, and move towards, the others within this distance.
     */
    float neighbour_radius = 3.0f;
    /**
     * @brief Members steer away from others closer than this.
     */
    float separation_distance = 1.0f;
    /**
     * @brief How far above the ground members keep.
     */
    float ground_clearance = 1.0f;
};

/**
 * @class FlockController
 * @brief Simulates flocks with Reynolds' boids and draws each flock with a single instanced draw call.
 *
 * Every member steers to keep apart from close neighbours (separation), to swim the way its neighbours do (alignment),
 * and towards their center (cohesion); flocks also stay inside their box and above @ref FlockController::ground_height.
 *
 * The flock shader gets `bodyTransform` (turns, centers and scales the model into a body facing +Z) and `bodyLength`,
 * and reads each member's position and heading as the instance matrix (attribute 5).
 */
class FlockController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "FlockController";
    }

    void add_flock(const FlockSettings &settings, const resources::Model *model, const resources::Shader *shader);

    /**
     * @brief Height of the ground at a point on the XZ plane, which flocks keep above.
     */
    std::function<std::optional<float>(glm::vec2)> ground_height;

private:
    struct Member {
        glm::vec3 position;
        glm::vec3 velocity;
    };

    struct Flock {
        FlockSettings settings;
        const resources::Model *model;
        const resources::Shader *shader;
        glm::mat4 body_transform;
        std::vector<Member> members;
        /**
         * @brief Per member: position and orientation, mapping +Z to its swimming direction.
         */
        std::vector<glm::mat4> headings;
    };

    static constexpr float SEPARATION_WEIGHT = 1.5f;
    static constexpr float ALIGNMENT_WEIGHT  = 1.0f;
    static constexpr float COHESION_WEIGHT   = 0.4f;
    static constexpr float BOUNDS_WEIGHT     = 6.0f;
    static constexpr float GROUND_WEIGHT     = 10.0f;
    static constexpr float WANDER_WEIGHT     = 0.5f;
    /**
     * @brief Members turn back once they're further than this share of the extent from the center.
     */
    static constexpr float BOUNDS_MARGIN = 0.8f;
    /**
     * @brief Longest time step simulated at once, so a stalled frame doesn't throw the flock apart.
     */
    static constexpr float MAX_TIME_STEP = 0.05f;

    void update() override;
    void draw() override;

    void simulate(Flock &flock, float dt);
    [[nodiscard]] glm::vec3 steering(const Flock &flock, std::size_t member);
    [[nodiscard]] static glm::mat4 heading(const Member &member);
    [[nodiscard]] static glm::mat4 body_transform(const FlockSettings &settings, const resources::Model &model);

    std::vector<Flock> m_flocks;
    std::mt19937 m_random{1337};
};

}
