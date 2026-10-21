/**
 * @file Scene.hpp
 * @brief A scene of model placements, loaded from a JSON scene file.
 */

#pragma once

#include <engine/graphics/DrawBatch.hpp>
#include <engine/physics/HeightField.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {
class Renderer;

/**
 * @brief A model placed in the scene one or more times, drawn with one shader.
 */
struct SceneEntity {
    std::string model_name;
    const DrawBatch *batch                  = nullptr;
    const resources::Shader *shader         = nullptr;
    Effect effect                           = Effect::None;
    bool casts_shadow                       = false;
    /**
     * @brief Optional name of a group of entities that are shown and hidden together, see @ref Scene::set_group_visible.
     */
    std::string group;
    bool visible = true;
};

/**
 * @class Scene
 * @brief Every model placement in the scene, loaded once from a scene file and drawn in the order of the file.
 *
 * The scene file is the single source of truth for what the scene contains and how each entity is drawn:
 * @code
 * {
 *   "entities": [
 *     {
 *       "model": "pine_tree",           // name of the model in config.json
 *       "shader": "flower_shader",      // name of the shader in resources/shaders
 *       "method": "instanced",          // optional: "default" (default), "blended" or "instanced"
 *       "effect": "wind",               // optional: "none" (default), "wind" or "fire"
 *       "casts_shadow": true,           // optional, default false
 *       "collider": true,               // optional, default false: the entity is ground you can walk on, see Scene::ground
 *       "group": "night",               // optional, see Scene::set_group_visible
 *       "transforms": [
 *         // position in the world; optional rotation in degrees around x, y, z; optional scale, uniform or per axis
 *         {"position": [32, 17, 0], "rotation": [-90, 0, 0], "scale": 11}
 *       ]
 *     }
 *   ],
 *   // Optional particle emitters, drawn by the ParticleController; every field of ParticleEmitter has the same name here
 *   // (snake_case), "size" is [start, end], colors are [r, g, b, a], and "blend" is "additive" (default) or "alpha".
 *   "particles": [
 *     {"name": "embers", "group": "night", "position": [12, 18.6, 6.5], "count": 160, "lifetime": 2.5,
 *      "velocity": [0, 2.2, 0], "size": [0.05, 0.02], "color_start": [6, 2.4, 0.6, 1], "color_end": [1.5, 0.2, 0.05, 0]}
 *   ],
 *   // Optional flocks (e.g. schools of fish), simulated and drawn by the FlockController; every field of FlockSettings has the
 *   // same name here, plus the "model" and "shader" to draw the members with.
 *   "flocks": [
 *     {"name": "reef_fish", "model": "fish", "shader": "fish", "count": 40, "center": [100, 1, 30], "extent": [18, 4, 18], "length": 0.4}
 *   ]
 * }
 * @endcode
 */
class Scene {
public:
    /**
     * @brief Loads the scene file and creates a @ref DrawBatch for every entity. Call after the engine is initialized.
     * @throws util::EngineError if the file is missing or malformed.
     */
    static Scene load(const std::filesystem::path &path);

    /**
     * @brief Draws the visible entities, in the order of the scene file.
     */
    void draw(Renderer &renderer) const;

    /**
     * @brief Draws the visible shadow casters into the current shadow cascade. Call from the renderer's depth scene callback.
     */
    void draw_shadow_casters(Renderer &renderer) const;

    /**
     * @brief Shows or hides the entities and particle emitters of `group`.
     */
    void set_group_visible(std::string_view group, bool visible);

    /**
     * @brief The walkable ground formed by all the collider entities.
     */
    [[nodiscard]] const physics::HeightField &ground() const {
        return m_ground;
    }

private:
    /**
     * @brief Spacing of the ground height samples, in world units.
     */
    static constexpr float GROUND_CELL_SIZE = 0.5f;

    std::vector<SceneEntity> m_entities;
    physics::HeightField m_ground;
    bool m_has_particles = false;
};

}
