/**
 * @file Image.hpp
 * @brief Decoded image pixels, loaded from a file or from memory. Decoding is thread-safe.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>

namespace engine::resources {

/**
 * @struct Image
 * @brief 8-bit pixels of a decoded image, in rows from the top (or from the bottom if loaded flipped).
 */
struct Image {
    struct PixelsDeleter {
        void operator()(uint8_t *pixels) const;
    };

    int32_t width{0};
    int32_t height{0};
    int32_t channels{0};
    std::unique_ptr<uint8_t, PixelsDeleter> pixels;
};

/**
 * @brief Decodes an image file (PNG, JPEG, BMP, TGA, ...). Safe to call from multiple threads at once.
 * @throws util::EngineError if the file can't be decoded.
 */
Image load_image(const std::filesystem::path &path, bool flip_vertically);

/**
 * @brief Decodes an image file that's already in memory. Safe to call from multiple threads at once.
 * @throws util::EngineError if the data can't be decoded.
 */
Image load_image_from_memory(std::span<const uint8_t> data, bool flip_vertically);

} // namespace engine::resources
