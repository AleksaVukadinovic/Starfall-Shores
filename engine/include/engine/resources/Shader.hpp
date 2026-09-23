/**
 * @file Shader.hpp
 * @brief Defines the Shader class that serves as an abstraction over OpenGL shaders.
 */

#pragma once

#include <engine/util/Utils.hpp>
#include <string>
#include <span>
#include <string_view>
#include <unordered_map>
#include <glm/glm.hpp>

namespace engine::resources {
using ShaderName = std::string;

/**
* @enum ShaderType
* @brief The type of the shader.
*/
enum class ShaderType {
    Vertex, Fragment, Geometry
};

/**
* @brief Converts a @ref ShaderType to a string.
* @param type The type of the shader.
* @returns The string representation of the shader type.
*/
std::string_view to_string(ShaderType type);

/**
* @class Shader
* @brief Represents a linked shader program object within the OpenGL context.
*/
class Shader {
    friend class ShaderCompiler;

public:
    /**
    * @brief Binds the shader program.
    */
    void use() const;

    /**
    * @brief Returns the OpenGL ID of the shader program.
    * @returns The OpenGL ID of the shader program.
    */
    [[nodiscard]] unsigned id() const;

    /**
    * @brief Sets a boolean uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_bool(std::string_view name, bool value) const;

    /**
    * @brief Sets an integer uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_int(std::string_view name, int value) const;

    /**
    * @brief Sets a float uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_float(std::string_view name, float value) const;

    /**
    * @brief Sets a 2D vector uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_vec2(std::string_view name, const glm::vec2 &value) const;

    /**
    * @brief Sets a 3D vector uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_vec3(std::string_view name, const glm::vec3 &value) const;

    /**
    * @brief Sets a 4D vector uniform value.
    * @param name The name of the uniform.
    * @param value The value to set.
    */
    void set_vec4(std::string_view name, const glm::vec4 &value) const;

    /**
    * @brief Sets a vec4 array uniform, starting at its first element.
    * @param name The name of the array uniform, without an index.
    * @param values The values to set.
    */
    void set_vec4_array(std::string_view name, std::span<const glm::vec4> values) const;

    /**
    * @brief Sets a 2x2 matrix uniform value.
    * @param name The name of the uniform.
    * @param mat The value to set.
    */
    void set_mat2(std::string_view name, const glm::mat2 &mat) const;

    /**
    * @brief Sets a 3x3 matrix uniform value.
    * @param name The name of the uniform.
    * @param mat The value to set.
    */
    void set_mat3(std::string_view name, const glm::mat3 &mat) const;

    /**
    * @brief Sets a 4x4 matrix uniform value.
    * @param name The name of the uniform.
    * @param mat The value to set.
    */
    void set_mat4(std::string_view name, const glm::mat4 &mat) const;

    /**
    * @brief Destroys the shader program in the OpenGL context.
    */
    void destroy() const;

    /**
    * @brief Returns the location of an active uniform, or -1 if the program has no such uniform
    * (setting a uniform at location -1 is a silent no-op in OpenGL).
    */
    [[nodiscard]] int32_t uniform_location(std::string_view name) const;

private:
    /**
    * @brief Constructs a Shader object from a linked program and applies the engine shader conventions.
    * @param shader_id The OpenGL ID of the shader program.
    * @param name The name of the shader program.
    * @param source The source code of the shader program.
    * @param source_path The path to the source file from which the shader program was compiled.
    */
    Shader(unsigned shader_id, std::string name, std::string source,
           std::filesystem::path source_path = "");

    /**
    * @brief Queries the locations of all active uniforms once, so setters never ask the driver.
    */
    void cache_uniform_locations();

    /**
    * @brief Binds texture samplers to their fixed texture units and the frame uniform block to its binding point.
    * See @ref engine/graphics/ShaderInterface.hpp.
    */
    void apply_conventions() const;

    struct StringHash {
        using is_transparent = void;

        std::size_t operator()(const std::string_view value) const {
            return std::hash<std::string_view>{}(value);
        }
    };

    /**
    * @brief The OpenGL ID of the shader program.
    */
    unsigned m_shader_id;

    /**
    * @brief The name of the shader program
    */
    std::string m_name;
    std::string m_source;
    std::filesystem::path m_source_path;
    std::unordered_map<std::string, int32_t, StringHash, std::equal_to<>> m_uniform_locations;
};
} // namespace engine


