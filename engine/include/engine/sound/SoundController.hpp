#pragma once

#include <engine/core/Controller.hpp>
#include <engine/resources/Sound.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

struct ma_engine;

namespace engine::sound {

class SoundController final : public core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "SoundController";
    }

    resources::Sound *sound(const std::string &name, const std::filesystem::path &path = "");

private:
    void initialize() override;
    void terminate() override;

    std::unique_ptr<ma_engine> m_engine;
    std::unordered_map<std::string, std::unique_ptr<resources::Sound>> m_sounds;

    const std::filesystem::path m_sounds_path = "resources/sounds";
};

} // namespace engine::sound
