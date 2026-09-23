#include <engine/resources/Image.hpp>
#include <engine/util/Errors.hpp>
#include <stb_image.h>
#include <format>

namespace engine::resources {

void Image::PixelsDeleter::operator()(uint8_t *pixels) const {
    stbi_image_free(pixels);
}

Image load_image(const std::filesystem::path &path, const bool flip_vertically) {
    // The per-thread flip setting keeps concurrent decodes from affecting each other.
    stbi_set_flip_vertically_on_load_thread(flip_vertically);
    Image image;
    image.pixels.reset(stbi_load(path.c_str(), &image.width, &image.height, &image.channels, 0));
    if (!image.pixels) {
        throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                std::format("Failed to load image {}: {}", path.string(), stbi_failure_reason()));
    }
    return image;
}

Image load_image_from_memory(const std::span<const uint8_t> data, const bool flip_vertically) {
    stbi_set_flip_vertically_on_load_thread(flip_vertically);
    Image image;
    image.pixels.reset(stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &image.width, &image.height,
                                             &image.channels, 0));
    if (!image.pixels) {
        throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                std::format("Failed to load an embedded image: {}", stbi_failure_reason()));
    }
    return image;
}

} // namespace engine::resources
