#pragma once
#include <cstdint>

namespace portal {
enum class Sound { Startup, FireBlue, FireOrange, InvalidSurface };
class AudioPort {
public:
    virtual ~AudioPort() = default;
    virtual bool play(Sound sound, std::uint32_t token) = 0;
    virtual void stop() = 0;
    virtual bool finished(std::uint32_t token) const = 0;
    virtual std::uint32_t durationMs(Sound sound) const = 0;
};
} // namespace portal
