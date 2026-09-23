/**
 * @file TextureLoader.hpp
 * @brief Loads many textures at once, decoding them in parallel.
 */

#pragma once

#include <engine/resources/Image.hpp>
#include <cstdint>
#include <functional>
#include <vector>

namespace engine::resources {

/**
 * @class TextureLoader
 * @brief Collects texture loads and performs them together: images are decoded on worker threads,
 * while the calling thread (which owns the OpenGL context) uploads each one as soon as it's decoded.
 */
class TextureLoader {
public:
    using Decoder = std::function<Image()>;

    /**
     * @brief Queues decoding an image into the already created OpenGL texture `texture_id`.
     * @param cost Rough decoding cost (e.g. the file size); the most expensive images are decoded first,
     * so a single large image doesn't end up alone at the end.
     */
    void enqueue(uint32_t texture_id, Decoder decode, std::uintmax_t cost);

    /**
     * @brief Decodes and uploads all queued textures, and blocks until they are done.
     * @throws The first error thrown by a decoder.
     */
    void load_all();

private:
    struct Job {
        uint32_t texture_id;
        Decoder decode;
        std::uintmax_t cost;
    };

    std::vector<Job> m_jobs;
};

} // namespace engine::resources
