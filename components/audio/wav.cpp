#include "portal/wav.hpp"
#include <algorithm>
#include <cstring>

namespace portal {
namespace {
std::uint16_t u16(const std::uint8_t* p) { return p[0] | (std::uint16_t{p[1]} << 8); }
std::uint32_t u32(const std::uint8_t* p) {
    return p[0] | (std::uint32_t{p[1]} << 8) | (std::uint32_t{p[2]} << 16) | (std::uint32_t{p[3]} << 24);
}
}
std::optional<WavView> parseWav(const std::uint8_t* bytes, std::size_t size) {
    if (!bytes || size < 12 || std::memcmp(bytes, "RIFF", 4) || std::memcmp(bytes + 8, "WAVE", 4))
        return std::nullopt;
    const auto riff_size = u32(bytes + 4);
    if (riff_size < 4 || riff_size > size - 8) return std::nullopt;
    const std::size_t end = static_cast<std::size_t>(riff_size) + 8;
    bool format = false;
    WavView view;
    for (std::size_t offset = 12; offset < end;) {
        if (end - offset < 8) return std::nullopt;
        const auto* header = bytes + offset;
        const auto chunk_size = u32(header + 4);
        offset += 8;
        if (chunk_size > end - offset) return std::nullopt;
        const auto* chunk = bytes + offset;
        if (!std::memcmp(header, "fmt ", 4)) {
            if (format || chunk_size < 16 || u16(chunk) != 1 || u16(chunk + 2) != 1 ||
                u32(chunk + 4) != tuning::AudioSampleRateHz || u32(chunk + 8) != tuning::AudioSampleRateHz * 2 ||
                u16(chunk + 12) != 2 || u16(chunk + 14) != 16) return std::nullopt;
            format = true;
        } else if (!std::memcmp(header, "data", 4)) {
            if (view.samples || !chunk_size || chunk_size % 2) return std::nullopt;
            view = {chunk, chunk_size / 2};
        }
        offset += chunk_size;
        if (chunk_size % 2) {
            if (offset == end) return std::nullopt;
            ++offset;
        }
    }
    return format && view.samples ? std::optional<WavView>{view} : std::nullopt;
}
std::int16_t pcmSample(const std::uint8_t* bytes, std::uint16_t gain_q15) {
    const std::int32_t raw = u16(bytes);
    const auto signed_sample = raw >= 32768 ? raw - 65536 : raw;
    return static_cast<std::int16_t>(signed_sample * std::min(gain_q15, std::uint16_t{32768}) / 32768);
}
} // namespace portal
