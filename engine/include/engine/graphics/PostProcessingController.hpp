#pragma once
#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformController.hpp>
#include <array>
#include <glm/glm.hpp>

namespace engine::resources {
class Shader;
}

namespace engine::graphics {
    /**
     * @class PostProcessingController
     * @brief Renders the scene into an HDR framebuffer, then applies underwater light shafts, bloom, tone mapping and greyscale.
     *
     * Bloom blurs the bright parts of the scene by progressively downsampling them through a chain of
     * half-resolution textures and additively upsampling them back ("dual filtering", as in Call of Duty: Advanced Warfare).
     * This produces a wide, smooth glow at a fraction of the cost of a full-resolution gaussian blur.
     */
    class PostProcessingController final : public core::Controller {
    public:
        static constexpr std::size_t BLOOM_MIP_COUNT = 6;

        float exposure            = 1.3f;
        bool bloom                = true;
        float bloom_strength      = 1.0f;
        /**
         * @brief Radius of the upsampling filter in texture coordinates; larger values spread the glow further.
         */
        float bloom_radius        = 0.005f;

        /**
         * @brief Strength of the sunbeams slanting down through the water. Set it while the camera is underwater, 0 otherwise.
         */
        float light_shafts = 0.0f;

        [[nodiscard]] std::string_view name() const override {
            return "PostProcessingController";
        }

    private:
        struct BloomMip {
            uint32_t texture{0};
            glm::ivec2 size{0};
        };

        void initialize() override;
        void begin_draw() override;
        void end_draw() override;
        void terminate() override;

        void update_screen_size();
        void create_hdr_attachments();
        void create_bloom_mips();
        void destroy_bloom_mips();
        void render_bloom() const;
        void render_final() const;
        void render_light_shafts() const;
        void render_quad() const;

        unsigned int m_hdr_fbo                  = 0;
        unsigned int m_color_buffers[2]         = {};
        unsigned int m_depth_texture            = 0;
        unsigned int m_composite_fbo            = 0;
        unsigned int m_bloom_fbo                = 0;
        std::array<BloomMip, BLOOM_MIP_COUNT> m_bloom_mips{};
        unsigned int m_scr_width                = 0;
        unsigned int m_scr_height               = 0;
        unsigned int m_quad_vao                 = 0;
        unsigned int m_quad_vbo                 = 0;

        const resources::Shader *m_downsample_shader = nullptr;
        const resources::Shader *m_upsample_shader   = nullptr;
        const resources::Shader *m_final_shader      = nullptr;
        const resources::Shader *m_light_shaft_shader = nullptr;
    };
}
