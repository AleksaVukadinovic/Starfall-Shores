#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/TextureLoader.hpp>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <exception>
#include <mutex>
#include <thread>

namespace engine::resources {

void TextureLoader::enqueue(const uint32_t texture_id, Decoder decode, const std::uintmax_t cost) {
    m_jobs.push_back({texture_id, std::move(decode), cost});
}

void TextureLoader::load_all() {
    std::ranges::sort(m_jobs, std::greater{}, &Job::cost);

    struct Decoded {
        std::size_t job;
        Image image;
        std::exception_ptr error;
    };
    std::mutex mutex;
    std::condition_variable decoded_ready;
    std::deque<Decoded> decoded;
    std::atomic<std::size_t> next_job{0};

    auto decode_jobs = [&](const std::stop_token &stop) {
        for (std::size_t job = next_job++; job < m_jobs.size() && !stop.stop_requested(); job = next_job++) {
            Decoded result{job};
            try {
                result.image = m_jobs[job].decode();
            } catch (...) {
                result.error = std::current_exception();
            }
            {
                std::lock_guard lock(mutex);
                decoded.push_back(std::move(result));
            }
            decoded_ready.notify_one();
        }
    };
    // Declared after the state they share, so the workers are joined before that state is destroyed.
    const std::size_t worker_count = std::min<std::size_t>(std::max(1u, std::thread::hardware_concurrency()), m_jobs.size());
    std::vector<std::jthread> workers;
    workers.reserve(worker_count);
    for (std::size_t i = 0; i < worker_count; ++i) {
        workers.emplace_back(decode_jobs);
    }

    for (std::size_t uploaded = 0; uploaded < m_jobs.size(); ++uploaded) {
        Decoded result;
        {
            std::unique_lock lock(mutex);
            decoded_ready.wait(lock, [&] { return !decoded.empty(); });
            result = std::move(decoded.front());
            decoded.pop_front();
        }
        if (result.error) {
            std::rethrow_exception(result.error);
        }
        graphics::OpenGL::upload_texture(m_jobs[result.job].texture_id, result.image);
    }
    m_jobs.clear();
}

} // namespace engine::resources
