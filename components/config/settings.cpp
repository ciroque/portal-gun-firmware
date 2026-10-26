#include "portal/settings.hpp"

namespace portal {
bool validSettings(const Settings& settings) {
    return static_cast<unsigned>(settings.color) < 2 &&
        static_cast<unsigned>(settings.curve) < 5 &&
        (settings.direction == -1 || settings.direction == 1);
}

ConfigBytes encodeSettings(const Settings& settings) {
    // Explicit bytes avoid compiler padding/enum-width dependencies. PORT, schema 1.
    return {0x54, 0x52, 0x4f, 0x50, 1, static_cast<std::uint8_t>(settings.color),
        static_cast<std::uint8_t>(settings.curve), static_cast<std::uint8_t>(settings.direction)};
}

bool decodeSettings(const std::uint8_t* bytes, std::size_t size, Settings& settings) {
    if (!bytes || size != 8 || bytes[0] != 0x54 || bytes[1] != 0x52 ||
        bytes[2] != 0x4f || bytes[3] != 0x50 || bytes[4] != 1 ||
        (bytes[7] != 1 && bytes[7] != 255)) return false;
    Settings candidate{static_cast<PortalColor>(bytes[5]), static_cast<Curve>(bytes[6]),
        static_cast<std::int8_t>(bytes[7] == 255 ? -1 : 1)};
    if (!validSettings(candidate)) return false;
    settings = candidate;
    return true;
}
} // namespace portal
