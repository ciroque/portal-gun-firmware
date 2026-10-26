#pragma once
#include "portal/tuning.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace portal {
struct WavView {
    const std::uint8_t* samples = nullptr;
    std::size_t sample_count = 0;
    std::uint32_t durationMs() const {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(sample_count) * 1000 + tuning::AudioSampleRateHz - 1) / tuning::AudioSampleRateHz);
    }
};
std::optional<WavView> parseWav(const std::uint8_t* bytes, std::size_t size);
std::int16_t pcmSample(const std::uint8_t* bytes, std::uint16_t gain_q15);
} // namespace portal
