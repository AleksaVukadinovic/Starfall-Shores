#include <engine/resources/Sound.hpp>
#include <engine/util/Errors.hpp>
#include <miniaudio.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace engine::resources {

Sound::Sound(ma_engine *engine, const std::filesystem::path &path, std::string name)
    : m_sound(new ma_sound)
    , m_path(path)
    , m_name(std::move(name)) {
    const ma_result result = ma_sound_init_from_file(engine, path.string().c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, m_sound);
    RG_GUARANTEE(result == MA_SUCCESS, "Failed to load sound: {} (error: {})", path.string(), static_cast<int>(result));
    spdlog::info(std::format("load_sound(name={}, path={})", m_name, m_path.string()));
}

Sound::~Sound() {
    if (m_sound) {
        ma_sound_uninit(m_sound);
        delete m_sound;
        m_sound = nullptr;
    }
}

Sound::Sound(Sound &&other) noexcept
    : m_sound(other.m_sound)
    , m_path(std::move(other.m_path))
    , m_name(std::move(other.m_name)) {
    other.m_sound = nullptr;
}

Sound &Sound::operator=(Sound &&other) noexcept {
    if (this != &other) {
        if (m_sound) {
            ma_sound_uninit(m_sound);
            delete m_sound;
        }
        m_sound = other.m_sound;
        m_path = std::move(other.m_path);
        m_name = std::move(other.m_name);
        other.m_sound = nullptr;
    }
    return *this;
}

void Sound::play() {
    RG_GUARANTEE(m_sound, "Sound not initialized: {}", m_name);
    ma_sound_seek_to_pcm_frame(m_sound, 0);
    ma_sound_start(m_sound);
}

void Sound::play_looping() {
    RG_GUARANTEE(m_sound, "Sound not initialized: {}", m_name);
    ma_sound_set_looping(m_sound, MA_TRUE);
    ma_sound_seek_to_pcm_frame(m_sound, 0);
    ma_sound_start(m_sound);
}

void Sound::stop() {
    RG_GUARANTEE(m_sound, "Sound not initialized: {}", m_name);
    ma_sound_stop(m_sound);
}

void Sound::set_volume(const float volume) {
    RG_GUARANTEE(m_sound, "Sound not initialized: {}", m_name);
    ma_sound_set_volume(m_sound, volume);
}

void Sound::set_looping(const bool looping) {
    RG_GUARANTEE(m_sound, "Sound not initialized: {}", m_name);
    ma_sound_set_looping(m_sound, looping ? MA_TRUE : MA_FALSE);
}

bool Sound::is_playing() const {
    return m_sound ? ma_sound_is_playing(m_sound) : false;
}

float Sound::volume() const {
    return m_sound ? ma_sound_get_volume(m_sound) : 0.0f;
}

} // namespace engine::resources
