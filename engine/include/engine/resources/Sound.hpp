#pragma once

#include <filesystem>
#include <string>

struct ma_sound;
struct ma_engine;

namespace engine::sound {
class SoundController;
}

namespace engine::resources {

class Sound {
    friend class sound::SoundController;

public:
    void play() const;
    void play_looping() const;
    void stop() const;

    void set_volume(float volume) const;
    void set_looping(bool looping) const;

    [[nodiscard]] bool is_playing() const;
    [[nodiscard]] float volume() const;

    [[nodiscard]] const std::filesystem::path &path() const {
        return m_path;
    }

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    ~Sound();

    Sound(const Sound &) = delete;
    Sound &operator=(const Sound &) = delete;
    Sound(Sound &&) noexcept;
    Sound &operator=(Sound &&) noexcept;

private:
    Sound(ma_engine *engine, const std::filesystem::path &path, std::string name);

    ma_sound *m_sound = nullptr;
    std::filesystem::path m_path;
    std::string m_name;
};

} // namespace engine::resources