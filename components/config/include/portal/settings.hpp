#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace portal {
enum class PortalColor : std::uint8_t { Blue, Orange };
enum class Curve : std::uint8_t { Linear, Quadratic, Cubic, Exponential, Smoothstep };
struct Settings {
    PortalColor color = PortalColor::Blue;
    Curve curve = Curve::Linear;
    std::int8_t direction = -1;
};
bool validSettings(const Settings& settings);
using ConfigBytes = std::array<std::uint8_t, 8>;
ConfigBytes encodeSettings(const Settings& settings);
bool decodeSettings(const std::uint8_t* bytes, std::size_t size, Settings& settings);
} // namespace portal
