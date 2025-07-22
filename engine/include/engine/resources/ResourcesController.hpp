/**
 * @file ResourcesController.hpp
 * @brief Defines the ResourcesController class  that manages resources such as models, textures, shaders, and skyboxes.
*/

#ifndef MATF_RG_PROJECT_RESOURCES_CONTROLLER_HPP
#define MATF_RG_PROJECT_RESOURCES_CONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Texture.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/resources/Skybox.hpp>
#include <unordered_map>

namespace engine::resources {
/**
* @class ResourcesController
* @brief Manages app resources: @ref Model, @ref Texture, @ref Shader, and @ref Skybox.
*/
class ResourcesController final : public core::Controller {
public:
    std::string_view name() const override {
        return "ResourcesController";
    }

    /**
    * @brief Retrieves the model with a given name. You are not supposed to call `delete` on this pointer.
    * @param name of the model in the configuration file.
    * @returns The pointer to the @ref Model associated with the `name`.
    */
    Model *model(const std::string &name);

    /**
    * @brief Retrieves the @ref Texture with a given name. You are not supposed to call `delete` on this pointer.
    *
    * Other params, except name, are optional. If not provided the function will search for a texture
    * in the: "resources/textures".
    * @param name of the texture without the extension.
    * @param path form which to load the texture.
    * @param texture_type
    * @param flip_uvs flip the uvs on load if set to true
    * @returns The pointer to the @ref Texture associated with the `name`.
    */
    Texture *texture(const std::string &name,
                     const std::filesystem::path &path = "",
                     TextureType texture_type = TextureType::Regular,
                     bool flip_uvs = false);

    /**
    * @brief Retrieves the @ref Skybox with a given name. You are not supposed to call `delete` on this pointer.
    *
    * Other params, except name, are optional. If not provided the function will search for a skybox
    * in the: "resources/skyboxes".
    * Images for the sides of the skybox cube should be named: left.jpg, right.jpg, top.jpg, bottom.jpg, front.jpg, back.jpg (any supported image extension).
    * @param name of the skybox directory that contains 6 images for each side of the cube.
    * @param path form which to load the texture.
    * @param flip_uvs flip the uvs on load if set to true
    * @returns The pointer to the @ref Skybox associated with the `name`.
    */
    Skybox *skybox(const std::string &name,
                   const std::filesystem::path &path = "", bool flip_uvs = false);

    /**
    * @brief Retrieves the @ref Shader with a given name. You are not supposed to call `delete` on this pointer.
    * @param name of the .glsl file in the `resources/shaders` directory
    * @param path to the shader.glsl file that contains shader source code.
    * @returns The pointer to the @ref Shader associated with the `name`.
    */
    Shader *shader(const std::string &name, const std::filesystem::path &path = "");

private:
    /**
    * @brief Loads all the resources from the "resources/" directory.
    */
    void initialize() override;

    /**
    * @brief Loads all the models from the "resources/models" directory based on the provided configuration. Called during @ref ResourcesController::initialize.
    */
    void load_models();

    /**
    * @brief Loads all the models from the "resources/textures" directory. Called during @ref ResourcesController::initialize.
    */
    void load_textures();

    /**
    * @brief Loads all the skyboxes from the "resources/skyboxes" directory. Called during @ref ResourcesController::initialize.
    */
    void load_skyboxes();

    /**
    * @brief Loads and compile all the shaders from the "resources/shaders" directory. Called during @ref ResourcesController::initialize.
    */
    void load_shaders();

    /**
    * @brief A hashmap of all the loaded @ref Model.
    */
    std::unordered_map<std::string, std::unique_ptr<Model> > m_models;
    /**
    * @brief A hashmap of all the loaded @ref Texture.
    */
    std::unordered_map<std::string, std::unique_ptr<Texture> > m_textures;
    /**
    * @brief A hashmap of all the loaded @ref Skybox.
    */
    std::unordered_map<std::string, std::unique_ptr<Skybox> > m_sky_boxes;
    /**
    * @brief A hashmap of all the loaded @ref Shader.
    */
    std::unordered_map<std::string, std::unique_ptr<Shader> > m_shaders;

    const std::filesystem::path m_models_path = "resources/models";
    const std::filesystem::path m_textures_path = "resources/textures";
    const std::filesystem::path m_shaders_path = "resources/shaders";
    const std::filesystem::path m_skyboxes_path = "resources/skyboxes";
};
} // namespace engine

#endif//MATF_RG_PROJECT_RESOURCES_CONTROLLER_HPP
