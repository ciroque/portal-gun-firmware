#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace portal {
/// Portal color selection persisted by its schema-defined ordinal.
enum class PortalColor : std::uint8_t { Blue, Orange };
/// Spin curve selection persisted by its schema-defined ordinal.
enum class Curve : std::uint8_t { Linear, Quadratic, Cubic, Exponential, Smoothstep };

/// User-selected effects; direction is restricted to -1 or +1.
struct Settings {
    PortalColor color = PortalColor::Blue;
    Curve curve = Curve::Linear;
    std::int8_t direction = -1;
};
/// Reject enum values and directions outside the supported settings domain.
bool validSettings(const Settings& settings);

/// Fixed-size, versioned storage representation used by the NVS adapter.
using ConfigBytes = std::array<std::uint8_t, 8>;
/// Encode settings into the schema-1 byte layout.
ConfigBytes encodeSettings(const Settings& settings);
/// Decode a complete valid blob. On failure, `settings` is left unchanged.
bool decodeSettings(const std::uint8_t* bytes, std::size_t size, Settings& settings);
} // namespace portal
