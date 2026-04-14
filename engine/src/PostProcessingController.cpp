#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glad/glad.h>

namespace engine::graphics {

    void PostProcessingController::render_quad() {
        if (m_quad_vao == 0) {
            float quadVertices[] = {
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
            CHECKED_GL_CALL(glGenBuffers, 1, &m_quad_vbo);
            CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
            CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
            CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
            CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0); // NOLINT
            CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
            CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float))); // NOLINT
        }
        CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
        CHECKED_GL_CALL(glBindVertexArray, 0);
    }

    void PostProcessingController::update_screen_size() {
        const auto platform = get<platform::PlatformController>();
        m_scr_width = platform->window()->width();
        m_scr_height = platform->window()->height();
        if (m_scr_width == 0)
            m_scr_width = 1400;
        if (m_scr_height == 0)
            m_scr_height = 1000;
    }

    void PostProcessingController::bloom_setup() {
        CHECKED_GL_CALL(glGenFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        CHECKED_GL_CALL(glGenTextures, 2, m_color_buffers);

        update_screen_size();
        for (uint8_t i = 0; i < 2; ++i) {
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[i]);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, m_scr_width, m_scr_height, 0, GL_RGBA, GL_FLOAT,
                            nullptr);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                            m_color_buffers[i], 0);
        }

        unsigned int rbo_depth;
        CHECKED_GL_CALL(glGenRenderbuffers, 1, &rbo_depth);
        CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbo_depth);
        CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_scr_width, m_scr_height);
        CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        CHECKED_GL_CALL(glDrawBuffers, 2, attachments);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

        CHECKED_GL_CALL(glGenFramebuffers, 2, m_pingpong_fbo);
        CHECKED_GL_CALL(glGenTextures, 2, m_pingpong_colorbuffers);

        for (uint8_t i = 0; i < 2; ++i) {
            CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_pingpong_fbo[i]);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_pingpong_colorbuffers[i]);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, m_scr_width, m_scr_height, 0, GL_RGBA, GL_FLOAT,
                            nullptr);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            m_pingpong_colorbuffers[i], 0);
        }
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

        const auto resources                 = get<resources::ResourcesController>();
        const resources::Shader *shader      = resources->shader("basic");
        const resources::Shader *blur_shader = resources->shader("blur");
        const resources::Shader *bloom_final = resources->shader("bloom_final");

        shader->use();
        shader->set_int("texture_diffuse1", 0);
        blur_shader->use();
        blur_shader->set_int("image", 0);
        bloom_final->use();
        bloom_final->set_int("scene", 0);
        bloom_final->set_int("bloomBlur", 1);
    }

    void PostProcessingController::render_bloom() {
        const auto resources = get<resources::ResourcesController>();

        const resources::Shader *blur_shader = resources->shader("blur");
        const resources::Shader *bloom_final = resources->shader("bloom_final");

        bool horizontal      = true;
        bool first_iteration = true;
        blur_shader->use();
        const unsigned int amount = bloom_passes;
        for (unsigned int i = 0; i < amount; ++i) {
            CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_pingpong_fbo[horizontal]);
            blur_shader->set_int("horizontal", horizontal);
            CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D,
                            first_iteration ? m_color_buffers[1] : m_pingpong_colorbuffers[!horizontal]);
            this->render_quad();
            horizontal      = !horizontal;
            first_iteration = false;
        }
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bloom_final->use();
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[0]);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_pingpong_colorbuffers[!horizontal]);

        bloom_final->set_bool("bloom", bloom);
        bloom_final->set_float("exposure", exposure);
        bloom_final->set_float("bloomStrength", bloom_strength);

        const auto greyscale = get<GreyscaleController>();
        bloom_final->set_bool("greyscaleEnabled", greyscale->greyscale_enabled);
        bloom_final->set_float("greyscaleStrength", greyscale->greyscale_strength);

        bloom_final->set_bool("underwaterEnabled", underwater);
        bloom_final->set_vec3("underwaterColor", underwater_color);
        bloom_final->set_float("underwaterIntensity", underwater_intensity);

        this->render_quad();
    }

    void PostProcessingController::prepare_hdr() {
        update_screen_size();

        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        CHECKED_GL_CALL(glViewport, 0, 0, m_scr_width, m_scr_height);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void PostProcessingController::finalize_bloom() {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        CHECKED_GL_CALL(glViewport, 0, 0, m_scr_width, m_scr_height);
        render_bloom();
    }

    void PostProcessingController::terminate() {
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glDeleteFramebuffers, 2, m_pingpong_fbo);

        CHECKED_GL_CALL(glDeleteTextures, 2, m_color_buffers);
        CHECKED_GL_CALL(glDeleteTextures, 2, m_pingpong_colorbuffers);

        if (m_quad_vao != 0) {
            CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_quad_vao);
            m_quad_vao = 0;
        }

        if (m_quad_vbo != 0) {
            CHECKED_GL_CALL(glDeleteBuffers, 1, &m_quad_vbo);
            m_quad_vbo = 0;
        }
    }
}
