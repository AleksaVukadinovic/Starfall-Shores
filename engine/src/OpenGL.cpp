#include <glad/glad.h>
#include <filesystem>
#include <array>
#include <future>
#include <utility>
#include <vector>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>

namespace engine::graphics {
int32_t OpenGL::shader_type_to_opengl_type(const resources::ShaderType type) {
    switch (type) {
        case resources::ShaderType::Vertex: return GL_VERTEX_SHADER;
        case resources::ShaderType::Fragment: return GL_FRAGMENT_SHADER;
        case resources::ShaderType::Geometry: return GL_GEOMETRY_SHADER;
        default: RG_SHOULD_NOT_REACH_HERE("Unhandled ShaderType");
    }
}

uint32_t OpenGL::create_texture() {
    uint32_t texture_id = 0;
    CHECKED_GL_CALL(glGenTextures, 1, &texture_id);
    return texture_id;
}

void OpenGL::upload_texture(const uint32_t texture_id, const resources::Image &image) {
    const int32_t format = texture_format(image.channels);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, texture_id);
    // Rows of 1 and 3 channel images aren't necessarily 4-byte aligned.
    CHECKED_GL_CALL(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.pixels.get());
    CHECKED_GL_CALL(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
    CHECKED_GL_CALL(glGenerateMipmap, GL_TEXTURE_2D);

    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int32_t OpenGL::texture_format(int32_t number_of_channels) {
    switch (number_of_channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: RG_SHOULD_NOT_REACH_HERE("Unknown channels {}", number_of_channels);
    }
}

uint32_t OpenGL::init_skybox_cube() {
    static unsigned int skybox_vao = 0;
    if (skybox_vao != 0) {
        return skybox_vao;
    }
    float vertices[] = {
        #include <skybox_vertices.include>
    };
    uint32_t skybox_vbo = 0;
    CHECKED_GL_CALL(glGenVertexArrays, 1, &skybox_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &skybox_vbo);
    CHECKED_GL_CALL(glBindVertexArray, skybox_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, skybox_vbo);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0); // NOLINT
    return skybox_vao;
}

bool OpenGL::shader_compiled_successfully(const uint32_t shader_id) {
    int success;
    CHECKED_GL_CALL(glGetShaderiv, shader_id, GL_COMPILE_STATUS, &success);
    return success;
}

uint32_t OpenGL::compile_shader(const std::string &shader_source, const resources::ShaderType shader_type) {
    const uint32_t shader_id = CHECKED_GL_CALL(glCreateShader, shader_type_to_opengl_type(shader_type));
    const char *shader_source_cstr = shader_source.c_str();
    CHECKED_GL_CALL(glShaderSource, shader_id, 1, &shader_source_cstr, nullptr);
    CHECKED_GL_CALL(glCompileShader, shader_id);
    return shader_id;
}

std::string OpenGL::get_compilation_error_message(const uint32_t shader_id) {
    char infoLog[512];
    CHECKED_GL_CALL(glGetShaderInfoLog, shader_id, 512, nullptr, infoLog);
    return infoLog;
}

std::string_view gl_call_error_description(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return
                    "GL_NO_ERROR: No error has been recorded. The value of this symbolic constant is guaranteed to be 0. ";
        case GL_INVALID_ENUM:
            return
                    "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_VALUE:
            return
                    "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_OPERATION:
            return
                    "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return
                    "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete."
                    "The offending command is ignored and has no other side effect than to set the error flag.";
        case GL_OUT_OF_MEMORY:
            return
                    "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded. . ";
        default: return "No Description";
    }
}

void OpenGL::assert_no_error(const std::source_location location) {
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        throw util::EngineError(util::EngineError::Type::OpenGLError,
                                std::format("OpenGL call error: '{}'", gl_call_error_description(error)), location);
    };
}

uint32_t face_index(std::string_view name);

uint32_t OpenGL::load_skybox_textures(const std::filesystem::path &path, const bool flip_uvs) {
    RG_GUARANTEE(std::filesystem::is_directory(path),
                 "Directory '{}' doesn't exist. Please specify path to be a directory to where the cubemap textures are located. The cubemap textures should be named: right, left, top, bottom, front, back; by their respective faces in the cubemap.",
                 path.string());
    std::vector<std::pair<std::filesystem::path, std::future<resources::Image>>> faces;
    for (const auto &file: std::filesystem::directory_iterator(path)) {
        faces.emplace_back(file.path(), std::async(std::launch::async, [file_path = file.path(), flip_uvs] {
            return resources::load_image(file_path, flip_uvs);
        }));
    }

    uint32_t texture_id;
    CHECKED_GL_CALL(glGenTextures, 1, &texture_id);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, texture_id);
    CHECKED_GL_CALL(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    for (auto &[face_path, decoding]: faces) {
        const resources::Image face = decoding.get();
        const uint32_t i = face_index(face_path.stem().c_str());
        const int32_t format = texture_format(face.channels);
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, face.width, face.height, 0, format, GL_UNSIGNED_BYTE, face.pixels.get());
    }
    CHECKED_GL_CALL(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture_id;
}

void OpenGL::enable_depth_testing() {
    CHECKED_GL_CALL(glEnable, GL_DEPTH_TEST);
}

void OpenGL::disable_depth_testing() {
    CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
}

void OpenGL::clear_buffers() {
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGL::enable_blending() {
    CHECKED_GL_CALL(glEnable, GL_BLEND);
    CHECKED_GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGL::disable_blending() {
    CHECKED_GL_CALL(glDisable, GL_BLEND);
}

OpenGL::ShadowMap OpenGL::create_shadow_map(const uint32_t size, const uint32_t layers) {
    uint32_t fbo, texture;
    CHECKED_GL_CALL(glGenFramebuffers, 1, &fbo);
    CHECKED_GL_CALL(glGenTextures, 1, &texture);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, texture);
    CHECKED_GL_CALL(glTexImage3D, GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, size, size, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    constexpr float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    CHECKED_GL_CALL(glTexParameterfv, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
    CHECKED_GL_CALL(glFramebufferTextureLayer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0, 0);
    CHECKED_GL_CALL(glDrawBuffer, GL_NONE);
    CHECKED_GL_CALL(glReadBuffer, GL_NONE);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    return {fbo, texture};
}

void OpenGL::begin_shadow_pass(const ShadowMap &shadow_map, const uint32_t size, const uint32_t layer) {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, shadow_map.fbo);
    CHECKED_GL_CALL(glFramebufferTextureLayer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map.texture, 0, static_cast<int>(layer));
    CHECKED_GL_CALL(glViewport, 0, 0, static_cast<int>(size), static_cast<int>(size));
    CHECKED_GL_CALL(glClear, GL_DEPTH_BUFFER_BIT);
}

void OpenGL::end_shadow_pass(const uint32_t screen_width, const uint32_t screen_height) {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    CHECKED_GL_CALL(glViewport, 0, 0, static_cast<int>(screen_width), static_cast<int>(screen_height));
}

void OpenGL::bind_texture_array_to_unit(const uint32_t texture_id, const uint32_t unit) {
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + unit);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, texture_id);
}

void OpenGL::destroy_shadow_map(const ShadowMap &shadow_map) {
    CHECKED_GL_CALL(glDeleteFramebuffers, 1, &shadow_map.fbo);
    CHECKED_GL_CALL(glDeleteTextures, 1, &shadow_map.texture);
}

uint32_t face_index(std::string_view name) {
    if (name == "right")  return 0;
    if (name == "left")   return 1;
    if (name == "top")    return 2;
    if (name == "bottom") return 3;
    if (name == "front")  return 4;
    if (name == "back")   return 5;
    RG_SHOULD_NOT_REACH_HERE(
            "Unknown face name: {}. The cubemap textures should be named: right, left, top, bottom, front, back; by their respective faces in the cubemap. The extension of the image file is ignored.", name);
}

int32_t stbi_number_of_channels_to_gl_format(int32_t number_of_channels) {
    switch (number_of_channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: RG_SHOULD_NOT_REACH_HERE("Unknown channels {}", number_of_channels);
    }
}

};
