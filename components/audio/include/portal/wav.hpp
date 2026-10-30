#pragma once
#include "portal/tuning.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace portal {
/// Non-owning view into validated, little-endian mono PCM sample bytes.
struct WavView {
    /// Pointer to the first PCM sample, owned by the embedded WAV asset.
    const std::uint8_t* samples = nullptr;
    /// Number of mono samples available from `samples`.
    std::size_t sample_count = 0;
    /// Rounded-up duration at the configured I2S sample rate.
    std::uint32_t durationMs() const {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(sample_count) * 1000 + tuning::AudioSampleRateHz - 1) / tuning::AudioSampleRateHz);
    }
};
/// Accept only RIFF PCM, mono, 16-bit audio at the configured sample rate.
std::optional<WavView> parseWav(const std::uint8_t* bytes, std::size_t size);
/// Decode one signed little-endian PCM sample and apply bounded Q15 gain.
std::int16_t pcmSample(const std::uint8_t* bytes, std::uint16_t gain_q15);
} // namespace portal
