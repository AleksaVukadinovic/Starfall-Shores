#pragma once

#include <engine/core/Controller.hpp>
#include <engine/resources/Sound.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

struct ma_engine;

namespace engine::sound {
/**
 * @brief All sounds play through one group, which feeds a low-pass filter before the output. Defined in SoundController.cpp.
 */
struct AudioMixer;

class SoundController final : public core::Controller {
public:
    SoundController();
    ~SoundController() override;

    [[nodiscard]] std::string_view name() const override {
        return "SoundController";
    }

    resources::Sound *sound(const std::string &name, const std::filesystem::path &path = "");

    /**
     * @brief Muffles all sounds, like hearing them underwater: 0 = clear, 1 = fully muffled. Uses a low-pass filter.
     */
    void set_muffling(float amount);

private:
    static constexpr float CLEAR_CUTOFF_HZ    = 20000.0f;
    static constexpr float MUFFLED_CUTOFF_HZ  = 500.0f;
    static constexpr uint32_t FILTER_ORDER    = 2;

    void initialize() override;
    void terminate() override;

    std::unique_ptr<ma_engine> m_engine;
    std::unique_ptr<AudioMixer> m_mixer;
    float m_muffling = 0.0f;
    std::unordered_map<std::string, std::unique_ptr<resources::Sound>> m_sounds;

    const std::filesystem::path m_sounds_path = "resources/sounds";
};

} // namespace engine::sound
