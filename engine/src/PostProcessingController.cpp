#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/GreyscaleController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glad/glad.h>
#include <algorithm>

namespace engine::graphics {

    namespace {
        void set_linear_clamp_filtering() {
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }

    void PostProcessingController::initialize() {
        const auto platform = get<platform::PlatformController>();
        m_scr_width  = std::max(platform->window()->width(), 1);
        m_scr_height = std::max(platform->window()->height(), 1);

        CHECKED_GL_CALL(glGenFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glGenTextures, 2, m_color_buffers);
        CHECKED_GL_CALL(glGenRenderbuffers, 1, &m_rbo_depth);
        create_hdr_attachments();

        CHECKED_GL_CALL(glGenFramebuffers, 1, &m_bloom_fbo);
        create_bloom_mips();

        constexpr float quad_vertices[] = {
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
        CHECKED_GL_CALL(glGenBuffers, 1, &m_quad_vbo);
        CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
        CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
        CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0); // NOLINT
        CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
        CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float))); // NOLINT
        CHECKED_GL_CALL(glBindVertexArray, 0);

        const auto resources = get<resources::ResourcesController>();
        m_downsample_shader  = resources->shader("bloom_downsample");
        m_upsample_shader    = resources->shader("bloom_upsample");
        m_final_shader       = resources->shader("bloom_final");
        m_final_shader->use();
        m_final_shader->set_int("scene", 0);
        m_final_shader->set_int("bloomBlur", 1);
    }

    void PostProcessingController::create_hdr_attachments() {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        for (uint32_t i = 0; i < 2; ++i) {
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[i]);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, m_scr_width, m_scr_height, 0, GL_RGBA, GL_FLOAT, nullptr);
            set_linear_clamp_filtering();
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_color_buffers[i], 0);
        }
        CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, m_rbo_depth);
        CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_scr_width, m_scr_height);
        CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_depth);

        constexpr unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        CHECKED_GL_CALL(glDrawBuffers, 2, attachments);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    }

    void PostProcessingController::create_bloom_mips() {
        glm::ivec2 size(m_scr_width, m_scr_height);
        for (auto &mip: m_bloom_mips) {
            size     = glm::max(size / 2, glm::ivec2(1));
            mip.size = size;
            CHECKED_GL_CALL(glGenTextures, 1, &mip.texture);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, mip.texture);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, size.x, size.y, 0, GL_RGB, GL_FLOAT, nullptr);
            set_linear_clamp_filtering();
        }
    }

    void PostProcessingController::destroy_bloom_mips() {
        for (auto &mip: m_bloom_mips) {
            CHECKED_GL_CALL(glDeleteTextures, 1, &mip.texture);
            mip = {};
        }
    }

    void PostProcessingController::update_screen_size() {
        const auto platform   = get<platform::PlatformController>();
        const unsigned int w  = std::max(platform->window()->width(), 1);
        const unsigned int h  = std::max(platform->window()->height(), 1);
        if (w != m_scr_width || h != m_scr_height) {
            m_scr_width  = w;
            m_scr_height = h;
            create_hdr_attachments();
            destroy_bloom_mips();
            create_bloom_mips();
        }
    }

    void PostProcessingController::render_quad() const {
        CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
    }

    void PostProcessingController::render_bloom() const {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_bloom_fbo);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);

        // Downsample the bright pixels through the mip chain.
        m_downsample_shader->use();
        uint32_t source = m_color_buffers[1];
        glm::ivec2 source_size(m_scr_width, m_scr_height);
        for (std::size_t i = 0; i < BLOOM_MIP_COUNT; ++i) {
            const BloomMip &mip = m_bloom_mips[i];
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
            CHECKED_GL_CALL(glViewport, 0, 0, mip.size.x, mip.size.y);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, source);
            m_downsample_shader->set_vec2("sourceTexelSize", 1.0f / glm::vec2(source_size));
            // Karis average on the full resolution input suppresses flickering from single very bright pixels.
            m_downsample_shader->set_bool("karisAverage", i == 0);
            render_quad();
            source      = mip.texture;
            source_size = mip.size;
        }

        // Upsample back up the chain, adding each blurred level onto the next larger one.
        m_upsample_shader->use();
        m_upsample_shader->set_float("filterRadius", bloom_radius);
        CHECKED_GL_CALL(glEnable, GL_BLEND);
        CHECKED_GL_CALL(glBlendFunc, GL_ONE, GL_ONE);
        for (std::size_t i = BLOOM_MIP_COUNT - 1; i > 0; --i) {
            const BloomMip &target = m_bloom_mips[i - 1];
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);
            CHECKED_GL_CALL(glViewport, 0, 0, target.size.x, target.size.y);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_bloom_mips[i].texture);
            render_quad();
        }
        CHECKED_GL_CALL(glDisable, GL_BLEND);
    }

    void PostProcessingController::render_final() const {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        CHECKED_GL_CALL(glViewport, 0, 0, m_scr_width, m_scr_height);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_final_shader->use();
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_color_buffers[0]);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_bloom_mips[0].texture);

        m_final_shader->set_bool("bloom", bloom);
        m_final_shader->set_float("exposure", exposure);
        // Every mip level adds its own blur onto mip 0, so normalize by the number of levels.
        m_final_shader->set_float("bloomStrength", bloom_strength / static_cast<float>(BLOOM_MIP_COUNT));

        const auto greyscale = get<GreyscaleController>();
        m_final_shader->set_bool("greyscaleEnabled", greyscale->greyscale_enabled);
        m_final_shader->set_float("greyscaleStrength", greyscale->greyscale_strength);

        m_final_shader->set_bool("underwaterEnabled", underwater);
        m_final_shader->set_vec3("underwaterColor", underwater_color);
        m_final_shader->set_float("underwaterIntensity", underwater_intensity);

        render_quad();
    }

    void PostProcessingController::begin_draw() {
        get<GraphicsController>()->profiler().begin_section("Scene");
        update_screen_size();
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_hdr_fbo);
        CHECKED_GL_CALL(glViewport, 0, 0, m_scr_width, m_scr_height);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void PostProcessingController::end_draw() {
        get<GraphicsController>()->profiler().begin_section("Post-processing");
        if (bloom) {
            render_bloom();
        }
        render_final();
    }

    void PostProcessingController::terminate() {
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_hdr_fbo);
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_bloom_fbo);
        CHECKED_GL_CALL(glDeleteTextures, 2, m_color_buffers);
        CHECKED_GL_CALL(glDeleteRenderbuffers, 1, &m_rbo_depth);
        destroy_bloom_mips();
        CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_quad_vao);
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_quad_vbo);
    }
}
