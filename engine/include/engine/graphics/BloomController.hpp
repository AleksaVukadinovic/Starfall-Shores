#ifndef BLOOMCONTROLLER_HPP
#define BLOOMCONTROLLER_HPP
#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformController.hpp>

namespace engine::graphics {
    class BloomController final : public core::Controller {
    public:
        int bloom_passes     = 10;
        float exposure       = 1.3f;
        bool bloom           = true;
        float bloom_strength = 1.0f;

        void render_bloom();

        void prepare_hdr();

        void finalize_bloom();

        void bloom_setup();

        [[nodiscard]] std::string_view name() const override {
            return "app::BloomController";
        }

        void terminate() override;

    private:
        unsigned int m_pingpong_FBO[2]          = {};
        unsigned int m_pingpong_colorbuffers[2] = {};
        unsigned int m_hdr_FBO                  = 0;
        unsigned int m_color_buffers[2]         = {};
        unsigned int m_scr_width                  = 0;
        unsigned int m_scr_height                 = 0;
        unsigned int m_quadVAO                  = 0;
        unsigned int m_quadVBO                  = 0;

        void render_quad();
    };
}

#endif //BLOOMCONTROLLER_HPP
