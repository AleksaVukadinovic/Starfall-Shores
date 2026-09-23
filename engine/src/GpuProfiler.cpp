#include <glad/glad.h>
#include <engine/graphics/GpuProfiler.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/util/Errors.hpp>
#include <algorithm>

namespace engine::graphics {

void GpuProfiler::initialize() {
    for (auto &frame: m_frames) {
        CHECKED_GL_CALL(glGenQueries, static_cast<int>(MAX_SECTIONS), frame.queries.data());
    }
}

void GpuProfiler::destroy() {
    for (auto &frame: m_frames) {
        CHECKED_GL_CALL(glDeleteQueries, static_cast<int>(MAX_SECTIONS), frame.queries.data());
    }
}

void GpuProfiler::begin_section(const std::string_view name) {
    end_section();
    FrameQueries &frame = m_frames[m_frame_index];
    RG_GUARANTEE(frame.count < MAX_SECTIONS, "Too many profiler sections in a single frame, the maximum is {}.", MAX_SECTIONS);
    frame.names[frame.count] = name;
    CHECKED_GL_CALL(glBeginQuery, GL_TIME_ELAPSED, frame.queries[frame.count]);
    ++frame.count;
    m_section_open  = true;
    m_section_start = Clock::now();
}

void GpuProfiler::end_section() {
    if (!m_section_open) {
        return;
    }
    CHECKED_GL_CALL(glEndQuery, GL_TIME_ELAPSED);
    const FrameQueries &frame = m_frames[m_frame_index];
    const float cpu_ms        = std::chrono::duration<float, std::milli>(Clock::now() - m_section_start).count();
    SectionTiming &section    = timing(frame.names[frame.count - 1]);
    section.cpu_ms += (cpu_ms - section.cpu_ms) * SMOOTHING;
    m_section_open = false;
}

void GpuProfiler::end_frame() {
    end_section();
    decay_skipped_sections(m_frames[m_frame_index], &SectionTiming::cpu_ms);
    m_frame_index = (m_frame_index + 1) % FRAME_LATENCY;
    collect_gpu_results(m_frames[m_frame_index]);
}

void GpuProfiler::decay_skipped_sections(const FrameQueries &frame, float SectionTiming::*value) {
    // Sections that were skipped in a frame (e.g. shadows that didn't need an update) decay towards zero,
    // so the displayed time is the cost amortized over frames.
    const auto recorded_names = std::span(frame.names).first(frame.count);
    for (auto &section: m_timings) {
        if (std::ranges::find(recorded_names, section.name) == recorded_names.end()) {
            section.*value -= section.*value * SMOOTHING;
        }
    }
}

void GpuProfiler::collect_gpu_results(FrameQueries &frame) {
    decay_skipped_sections(frame, &SectionTiming::gpu_ms);
    for (std::size_t i = 0; i < frame.count; ++i) {
        int32_t available = 0;
        CHECKED_GL_CALL(glGetQueryObjectiv, frame.queries[i], GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available) {
            continue;
        }
        uint64_t elapsed_ns = 0;
        CHECKED_GL_CALL(glGetQueryObjectui64v, frame.queries[i], GL_QUERY_RESULT, &elapsed_ns);
        SectionTiming &section = timing(frame.names[i]);
        section.gpu_ms += (static_cast<float>(elapsed_ns) * 1e-6f - section.gpu_ms) * SMOOTHING;
    }
    frame.count = 0;
}

GpuProfiler::SectionTiming &GpuProfiler::timing(const std::string_view name) {
    const auto it = std::ranges::find(m_timings, name, &SectionTiming::name);
    if (it != m_timings.end()) {
        return *it;
    }
    return m_timings.emplace_back(SectionTiming{name});
}

} // namespace engine::graphics
