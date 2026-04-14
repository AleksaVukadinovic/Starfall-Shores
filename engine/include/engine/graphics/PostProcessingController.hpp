#pragma once
#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformController.hpp>
#include <glm/glm.hpp>

namespace engine::graphics {
    class PostProcessingController final : public core::Controller {
    public:
        unsigned int bloom_passes = 10;
        float exposure            = 1.3f;
        bool bloom                = true;
        float bloom_strength      = 1.0f;

        bool underwater              = false;
        glm::vec3 underwater_color   = glm::vec3(0.0f, 0.3f, 0.5f);
        float underwater_intensity   = 0.4f;

        void prepare_hdr();

        void finalize_bloom();

        void bloom_setup();

        [[nodiscard]] std::string_view name() const override {
            return "app::BloomController";
        }

        void terminate() override;

    private:
        unsigned int m_pingpong_fbo[2]          = {};
        unsigned int m_pingpong_colorbuffers[2] = {};
        unsigned int m_hdr_fbo                  = 0;
        unsigned int m_color_buffers[2]         = {};
        unsigned int m_scr_width                  = 0;
        unsigned int m_scr_height                 = 0;
        unsigned int m_quad_vao                  = 0;
        unsigned int m_quad_vbo                  = 0;

        void update_screen_size();
        void render_bloom();
        void render_quad();
    };
}
