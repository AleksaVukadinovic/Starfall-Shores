#include <glad/glad.h>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Errors.hpp>
#include <format>
#include <spdlog/spdlog.h>
#include <engine/graphics/OpenGL.hpp>

namespace engine::resources {
using namespace graphics;

int to_opengl_type(ShaderType type);

Shader ShaderCompiler::compile_from_source(std::string shader_name, std::string shader_source) {
    spdlog::info("ShaderCompiler::Compiling: {}", shader_name);
    ShaderCompiler compiler(std::move(shader_name), std::move(shader_source));
    ShaderParsingResult parsing_result = compiler.parse_source();
    OpenGL::ShaderProgramId shader_program = compiler.compile(parsing_result);
    Shader result(shader_program, shader_name, shader_source, "");
    return result;
}

OpenGL::ShaderProgramId ShaderCompiler::compile(const ShaderParsingResult &shader_sources) {
    uint32_t shader_program_id = glCreateProgram();
    uint32_t vertex_shader_id = 0;
    uint32_t fragment_shader_id = 0;
    uint32_t geometry_shader_id = 0;
    defer {
        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);
        glDeleteShader(geometry_shader_id);
    };

    vertex_shader_id = compile(shader_sources.vertex_shader, ShaderType::Vertex);
    glAttachShader(shader_program_id, vertex_shader_id);
    fragment_shader_id = compile(shader_sources.fragment_shader, ShaderType::Fragment);
    glAttachShader(shader_program_id, fragment_shader_id);

    if (!shader_sources.geometry_shader
                       .empty()) {
        geometry_shader_id = compile(shader_sources.geometry_shader, ShaderType::Geometry);
        glAttachShader(shader_program_id, geometry_shader_id);
    }
    glLinkProgram(shader_program_id);
    return shader_program_id;
}

uint32_t ShaderCompiler::compile(const std::string &shader_source, ShaderType type) {
    uint32_t shader_id = OpenGL::compile_shader(shader_source, type);
    if (!OpenGL::shader_compiled_successfully(shader_id)) {
        throw util::EngineError(util::EngineError::Type::ShaderCompilationError, std::format(
                "{} shader compilation {} failed:\n{}", to_string(type),
                m_shader_name,
                OpenGL::get_compilation_error_message(shader_id)));
    }
    return shader_id;
}

ShaderParsingResult ShaderCompiler::parse_source() {
    ShaderParsingResult parsing_result;
    std::istringstream ss(m_sources);
    std::string line;
    std::string *current_shader = nullptr;
    while (std::getline(ss, line)) {
        if (line.starts_with("//#shader") || line.starts_with("// #shader")) {
            current_shader = now_parsing(parsing_result, line);
        } else if (current_shader) {
            current_shader->append(line);
            current_shader->push_back('\n');
        }
    }
    if (parsing_result.vertex_shader
                      .empty() || parsing_result.fragment_shader
                                                .empty()) {
        throw util::EngineError(util::EngineError::Type::ShaderCompilationError, std::format(
                "Error compiling: {}. Source for vertex and fragment shader must be defined. Vertex shader source must begin with: '//#shader vertex'; and fragment shader source must begin with: '//#shader fragment'",
                m_shader_name));
    }
    return parsing_result;
}

Shader ShaderCompiler::compile_from_file(std::string shader_name,
                                         const std::filesystem::path &shader_path) {
    if (!exists(shader_path)) {
        throw util::EngineError(util::EngineError::Type::FileNotFound,
                                std::format("Shader source file {} for shader {} not found.",
                                            shader_path.string(),
                                            shader_name));
    }
    std::string shader_source = util::read_text_file(shader_path);
    ShaderCompiler compiler(std::move(shader_name), std::move(shader_source));
    ShaderParsingResult parsing_result = compiler.parse_source();
    OpenGL::ShaderProgramId shader_program = compiler.compile(parsing_result);
    Shader result(shader_program, shader_name, shader_source, shader_path);
    return result;
}

std::string *ShaderCompiler::now_parsing(ShaderParsingResult &result, const std::string &line) {
    if (line.ends_with(to_string(ShaderType::Vertex))) {
        return &result.vertex_shader;
    }
    if (line.ends_with(to_string(ShaderType::Fragment))) {
        return &result.fragment_shader;
    }
    if (line.ends_with(to_string(ShaderType::Geometry))) {
        return &result.geometry_shader;
    }
    RG_SHOULD_NOT_REACH_HERE("Unknown type of shader prefix: {}. Did you mean: #shader {}|{}|{}", line,
                             to_string(ShaderType::Vertex), to_string(ShaderType::Fragment),
                             to_string(ShaderType::Geometry));
}

std::string_view to_string(ShaderType type) {
    switch (type) {
        case ShaderType::Vertex: return "vertex";
        case ShaderType::Fragment: return "fragment";
        case ShaderType::Geometry: return "geometry";
        default: RG_SHOULD_NOT_REACH_HERE("Unhandled shader type");
    }
}

}
