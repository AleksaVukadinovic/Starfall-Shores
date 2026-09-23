/**
 * @file GpuProfiler.hpp
 * @brief Measures the CPU and GPU time of consecutive sections of a frame.
 */

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace engine::graphics {

/**
 * @class GpuProfiler
 * @brief Splits every frame into named, consecutive sections and measures each one.
 *
 * Calling @ref GpuProfiler::begin_section closes the currently open section and opens a new one,
 * so sections never nest (OpenGL timer queries can't nest either). GPU results are read back
 * a few frames later to avoid stalling the pipeline, and all timings are smoothed over time.
 *
 * Section names must outlive the profiler; use string literals.
 */
class GpuProfiler {
public:
    struct SectionTiming {
        std::string_view name;
        float gpu_ms{0.0f};
        float cpu_ms{0.0f};
    };

    static constexpr std::size_t MAX_SECTIONS = 8;

    void initialize();

    void destroy();

    void begin_section(std::string_view name);

    /**
     * @brief Closes the last section of the frame. Call once per frame, before swapping buffers.
     */
    void end_frame();

    [[nodiscard]] std::span<const SectionTiming> timings() const {
        return m_timings;
    }

private:
    using Clock = std::chrono::steady_clock;

    /**
     * @brief Queries in flight for a single frame.
     */
    struct FrameQueries {
        std::array<uint32_t, MAX_SECTIONS> queries{};
        std::array<std::string_view, MAX_SECTIONS> names{};
        std::size_t count{0};
    };

    /**
     * @brief Number of frames a GPU result is allowed to lag behind; 3 is enough for the driver to finish a frame.
     */
    static constexpr std::size_t FRAME_LATENCY = 3;
    static constexpr float SMOOTHING = 0.05f;

    void end_section();

    void collect_gpu_results(FrameQueries &frame);

    void decay_skipped_sections(const FrameQueries &frame, float SectionTiming::*value);

    SectionTiming &timing(std::string_view name);

    std::array<FrameQueries, FRAME_LATENCY> m_frames{};
    std::size_t m_frame_index{0};
    bool m_section_open{false};
    Clock::time_point m_section_start{};
    std::vector<SectionTiming> m_timings;
};

} // namespace engine::graphics
