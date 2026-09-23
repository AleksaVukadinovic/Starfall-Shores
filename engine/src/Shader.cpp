#include <glad/glad.h>
#include <engine/resources/Shader.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/ShaderInterface.hpp>
#include <array>
#include <format>

namespace engine::resources {

void Shader::use() const {
    glUseProgram(m_shader_id);
}

void Shader::destroy() const {
    glDeleteProgram(m_shader_id);
}

unsigned Shader::id() const {
    return m_shader_id;
}

int32_t Shader::uniform_location(const std::string_view name) const {
    const auto it = m_uniform_locations.find(name);
    return it != m_uniform_locations.end() ? it->second : -1;
}

void Shader::set_bool(const std::string_view name, const bool value) const {
    CHECKED_GL_CALL(glUniform1i, uniform_location(name), static_cast<int>(value));
}

void Shader::set_int(const std::string_view name, const int value) const {
    CHECKED_GL_CALL(glUniform1i, uniform_location(name), value);
}

void Shader::set_float(const std::string_view name, float value) const {
    CHECKED_GL_CALL(glUniform1f, uniform_location(name), value);
}

void Shader::set_vec2(const std::string_view name, const glm::vec2 &value) const {
    CHECKED_GL_CALL(glUniform2fv, uniform_location(name), 1, &value[0]);
}

void Shader::set_vec3(const std::string_view name, const glm::vec3 &value) const {
    CHECKED_GL_CALL(glUniform3fv, uniform_location(name), 1, &value[0]);
}

void Shader::set_vec4(const std::string_view name, const glm::vec4 &value) const {
    CHECKED_GL_CALL(glUniform4fv, uniform_location(name), 1, &value[0]);
}

void Shader::set_mat2(const std::string_view name, const glm::mat2 &mat) const {
    CHECKED_GL_CALL(glUniformMatrix2fv, uniform_location(name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat3(const std::string_view name, const glm::mat3 &mat) const {
    CHECKED_GL_CALL(glUniformMatrix3fv, uniform_location(name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat4(const std::string_view name, const glm::mat4 &mat) const {
    CHECKED_GL_CALL(glUniformMatrix4fv, uniform_location(name), 1, GL_FALSE, &mat[0][0]);
}

Shader::Shader(unsigned shader_id, std::string name, std::string source, std::filesystem::path source_path) :
        m_shader_id(shader_id)
        , m_name(std::move(name))
        , m_source(std::move(source))
        , m_source_path(std::move(source_path)) {
    cache_uniform_locations();
    apply_conventions();
}

void Shader::cache_uniform_locations() {
    int32_t uniform_count = 0;
    CHECKED_GL_CALL(glGetProgramiv, m_shader_id, GL_ACTIVE_UNIFORMS, &uniform_count);
    std::array<char, 256> name_buffer{};
    for (int32_t i = 0; i < uniform_count; ++i) {
        int32_t length = 0, size = 0;
        uint32_t type  = 0;
        CHECKED_GL_CALL(glGetActiveUniform, m_shader_id, i, static_cast<int32_t>(name_buffer.size()), &length, &size, &type,
                        name_buffer.data());
        std::string uniform_name(name_buffer.data(), length);
        const int32_t location = CHECKED_GL_CALL(glGetUniformLocation, m_shader_id, uniform_name.c_str());
        if (location < 0) {
            continue; // members of uniform blocks have no location
        }
        // Arrays are reported as "name[0]"; make them reachable by their plain name too.
        if (uniform_name.ends_with("[0]")) {
            m_uniform_locations.emplace(uniform_name.substr(0, uniform_name.size() - 3), location);
        }
        m_uniform_locations.emplace(std::move(uniform_name), location);
    }
}

void Shader::apply_conventions() const {
    use();
    for (const auto type: {TextureType::Diffuse, TextureType::Specular, TextureType::Normal, TextureType::Height}) {
        const auto sampler_name = std::format("{}1", Texture::uniform_name_convention(type));
        set_int(sampler_name, static_cast<int>(graphics::texture_unit(type)));
    }
    set_int(graphics::SHADOW_MAP_SAMPLER, static_cast<int>(graphics::SHADOW_MAP_TEXTURE_UNIT));

    const uint32_t block_index = CHECKED_GL_CALL(glGetUniformBlockIndex, m_shader_id, graphics::FRAME_UNIFORMS_BLOCK.data());
    if (block_index != GL_INVALID_INDEX) {
        CHECKED_GL_CALL(glUniformBlockBinding, m_shader_id, block_index, graphics::FRAME_UNIFORMS_BINDING);
    }
}

}
