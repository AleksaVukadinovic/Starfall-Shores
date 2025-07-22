/**
 * @file ShaderCompiler.hpp
 * @brief Defines the ShaderCompiler class that serves as the interface for shader compilation.
*/

#ifndef SHADER_COMPILER_HPP
#define SHADER_COMPILER_HPP

#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>
#include <filesystem>
#include <string>

namespace engine::resources {
/**
* @struct ShaderParsingResult
* @brief Contains the parsed vertex, fragment, and geometry shaders, since the ShaderCompiler expects a single .glsl source file.
*/
struct ShaderParsingResult {
    std::string vertex_shader;
    std::string fragment_shader;
    std::string geometry_shader;
};

/**
* @class ShaderCompiler
* @brief Compiles GLSL shaders from a single source file.
* Vertex, Fragment and Geometry shaders are separated by the `// #shader vertex|fragment|geometry` directive.
* All the code following the directive belongs to the source of the shader specified in the `#shader` directive.
* Here is an example:
* @code
* //#shader vertex
* #version 330 core
* layout (location = 0) in vec3 aPos;
*
* void main() {
*     gl_Position = vec4(aPos, 1.0);
* }
*
* //#shader fragment
* #version 330 core
* out vec4 FragColor;
*
* void main() {
*     FragColor = vec4(0.0, 0.0, 0.0, 1.0);
* }
* @endcode
*/
class ShaderCompiler {
public:
    /**
    * @brief Compiles a shader from source.
    * @param shader_name
    * @param shader_source string for the vertex, fragment, [geometry] shader
    * @returns Compiled @ref Shader object that can be used for drawing.
    */
    static Shader compile_from_source(std::string shader_name, std::string shader_source);

    /**
    * @brief Compiles a shader from file.
    * @param shader_name
    * @param shader_path containing the source for the vertex, fragment, [geometry] shader
    * @returns Compiled @ref Shader object that can be used for drawing.
    */
    static Shader compile_from_file(std::string shader_name, const std::filesystem::path &shader_path);

    /**
    * @brief Splits a single shader source string into `vertex`, `fragment`, [`geometry`] shader strings.
    * @returns @ref ShaderParsingResult
    */
    ShaderParsingResult parse_source();

private:
    /**
    * @brief Compile shader sources into a OpenGL shader program.
    * @returns A @ref graphics::OpenGL::ShaderProgramId referencing OpenGL shader program.
    */
    graphics::OpenGL::ShaderProgramId compile(const ShaderParsingResult &shader_sources);

    ShaderCompiler(std::string shader_name, std::string shader_source) : m_shader_name(
            std::move(shader_name))
                                                                         , m_sources(std::move(shader_source)) {
    }

    /**
    * @brief Returns the output field from the @ref ShaderParsingResult for which to continue appending `line`s of the `m_sources`.
    * Detects if the line contains `//#shader` directive and returns a pointer to the appropriate
    * field in the @ref ShaderParsingResult.
    */
    std::string *now_parsing(ShaderParsingResult &result, const std::string &line);

    /**
    * @brief Compiles a single shader (Vertex|Fragment|Geometry) (not the whole program).
    * @param shader_source
    * @param type of the shader to compile Vertex|Fragment|Geometry
    * @returns OpenGL id to a compiled shader.
    */
    uint32_t compile(const std::string &shader_source, ShaderType type);

    std::string m_shader_name;
    std::string m_sources;
};
}
#endif //SHADER_COMPILER_HPP
