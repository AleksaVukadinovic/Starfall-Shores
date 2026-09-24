/**
 * @file Scene.hpp
 * @brief A scene of model placements, loaded from a JSON scene file.
 */

#pragma once

#include <engine/graphics/DrawBatch.hpp>
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
 *       "group": "night",               // optional, see Scene::set_group_visible
 *       "transforms": [
 *         // position in the world; optional rotation in degrees around x, y, z; optional scale, uniform or per axis
 *         {"position": [32, 17, 0], "rotation": [-90, 0, 0], "scale": 11}
 *       ]
 *     }
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

    void set_group_visible(std::string_view group, bool visible);

private:
    std::vector<SceneEntity> m_entities;
};

}
