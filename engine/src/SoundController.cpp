#include <engine/sound/SoundController.hpp>
#include <engine/util/Errors.hpp>
#include <miniaudio.h>
#include <spdlog/spdlog.h>

namespace engine::sound {

SoundController::SoundController() = default;
SoundController::~SoundController() = default;

void SoundController::initialize() {
    m_engine = std::make_unique<ma_engine>();
    const ma_result result = ma_engine_init(nullptr, m_engine.get());
    RG_GUARANTEE(result == MA_SUCCESS, "Failed to initialize audio engine (error: {})", static_cast<int>(result));
    spdlog::info("SoundController: audio engine initialized");
}

void SoundController::terminate() {
    m_sounds.clear();
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
        result = std::unique_ptr<resources::Sound>(new resources::Sound(m_engine.get(), sound_path, name));
    }
    return result.get();
}

} // namespace engine::sound
