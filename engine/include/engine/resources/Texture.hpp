/**
 * @file Texture.hpp
 * @brief Defines the Texture class that serves as an abstraction over OpenGL textures.
*/

#ifndef MATF_RG_PROJECT_TEXTURE_HPP
#define MATF_RG_PROJECT_TEXTURE_HPP

#include <string_view>
#include <filesystem>
#include <utility>

namespace engine::resources {
class Shader;

/**
* @enum TextureType used by the Model class to identify the type of the texture and bind it to the correct uniform sampler.
*/
enum class TextureType {
    Regular,
    Diffuse,
    Specular,
    Normal,
    Height,
};

/**
* @class Texture
* @brief Represents a texture object within the OpenGL context.
*/
class Texture {
    friend class ResourcesController;

public:
    /**
    * @brief Returns the uniform name convention for a given texture type. The convention is used by the Model class to bind the texture to the correct uniform sampler by name.
    * @param type The type of the texture.
    * @returns The uniform name convention for the texture type.
    * @code
    * // #shader fragment
    * uniform sampler2D texture_diffuse;
    * ...
    * void main() {
    *   gl_FragColor = texture(texture_diffuse, uv);
    * }
    * @endcode
    */
    static std::string_view uniform_name_convention(TextureType type);

    /**
    * @brief Destroys the texture object in the OpenGL context.
    */
    void destroy();

    /**
    * @brief Returns the type of the texture.
    * @returns The type of the texture.
    */
    TextureType type() const {
        return m_type;
    }

    /**
    * @brief Returns the OpenGL ID of the texture.
    * @returns The OpenGL ID of the texture.
    */
    uint32_t id() const {
        return m_id;
    }

    /**
    * @brief Binds the texture to a given sampler.
    * @param sampler The sampler to bind the texture to.
    */
    void bind(int32_t sampler);

    /**
    * @brief Returns the path to the texture file from which the texture was loaded.
    * @returns The path to the texture file.
    */
    const std::filesystem::path &path() const {
        return m_path;
    }

    /**
    * @brief Returns the name of the texture by which it can be referenced using the @ref engine::resources::ResourcesController::texture function.
    * @returns The name of the texture.
    */
    const std::string &name() const {
        return m_name;
    }

    Texture() = default;

private:
    /**
    * @brief Constructs a Texture object.
    * @param id The OpenGL ID of the texture.
    * @param type The type of the texture.
    * @param path The path to the texture file.
    * @param name The name of the texture.
    */
    Texture(uint32_t id, TextureType type, std::filesystem::path path, std::string name)
            : m_id(id)
              , m_type(type)
              , m_path(std::move(path))
              , m_name(std::move(name)) {
    }

    uint32_t m_id{};
    TextureType m_type{};
    std::filesystem::path m_path{};
    std::string m_name{};
};
} // namespace engine
#endif//MATF_RG_PROJECT_TEXTURE_HPP
