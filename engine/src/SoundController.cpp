#include <engine/sound/SoundController.hpp>
#include <engine/util/Errors.hpp>
#include <miniaudio.h>
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace engine::sound {

struct AudioMixer {
    ma_sound_group group;
    ma_lpf_node low_pass;
};

SoundController::SoundController() = default;
SoundController::~SoundController() = default;

void SoundController::initialize() {
    m_engine = std::make_unique<ma_engine>();
    const ma_result result = ma_engine_init(nullptr, m_engine.get());
    RG_GUARANTEE(result == MA_SUCCESS, "Failed to initialize audio engine (error: {})", static_cast<int>(result));

    // sounds -> group -> low-pass filter -> output
    m_mixer = std::make_unique<AudioMixer>();
    const ma_uint32 channels    = ma_engine_get_channels(m_engine.get());
    const ma_uint32 sample_rate = ma_engine_get_sample_rate(m_engine.get());
    const ma_lpf_node_config filter = ma_lpf_node_config_init(channels, sample_rate, CLEAR_CUTOFF_HZ, FILTER_ORDER);
    RG_GUARANTEE(ma_lpf_node_init(ma_engine_get_node_graph(m_engine.get()), &filter, nullptr, &m_mixer->low_pass) == MA_SUCCESS,
                 "Failed to create the audio low-pass filter");
    ma_node_attach_output_bus(&m_mixer->low_pass, 0, ma_engine_get_endpoint(m_engine.get()), 0);
    RG_GUARANTEE(ma_sound_group_init(m_engine.get(), 0, nullptr, &m_mixer->group) == MA_SUCCESS, "Failed to create the audio sound group");
    ma_node_attach_output_bus(&m_mixer->group, 0, &m_mixer->low_pass, 0);
    spdlog::info("SoundController: audio engine initialized");
}

void SoundController::terminate() {
    m_sounds.clear();
    if (m_mixer) {
        ma_sound_group_uninit(&m_mixer->group);
        ma_lpf_node_uninit(&m_mixer->low_pass, nullptr);
        m_mixer.reset();
    }
    if (m_engine) {
        ma_engine_uninit(m_engine.get());
    }
    m_engine.reset();
    spdlog::info("SoundController: audio engine terminated");
}

resources::Sound *SoundController::sound(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_sounds[name];
    if (!result) {
        const std::filesystem::path sound_path = path.empty() ? m_sounds_path / name : path;
        RG_GUARANTEE(exists(sound_path), "Sound file not found: {} (resolved path: {})", name, sound_path.string());
        result = std::unique_ptr<resources::Sound>(new resources::Sound(m_engine.get(), &m_mixer->group, sound_path, name));
    }
    return result.get();
}

void SoundController::set_muffling(const float amount) {
    if (amount == m_muffling) {
        return;
    }
    m_muffling = amount;
    // Interpolate the cutoff exponentially, since pitch is perceived logarithmically.
    const float cutoff = CLEAR_CUTOFF_HZ * std::pow(MUFFLED_CUTOFF_HZ / CLEAR_CUTOFF_HZ, std::clamp(amount, 0.0f, 1.0f));
    const ma_lpf_config filter = ma_lpf_config_init(ma_format_f32, ma_engine_get_channels(m_engine.get()),
                                                    ma_engine_get_sample_rate(m_engine.get()), cutoff, FILTER_ORDER);
    ma_lpf_node_reinit(&filter, &m_mixer->low_pass);
}

} // namespace engine::sound
