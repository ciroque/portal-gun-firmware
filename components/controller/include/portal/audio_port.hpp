#pragma once
#include <cstdint>

namespace portal {
/// Single-voice effects consumed by the controller.
enum class Sound { Startup, FireBlue, FireOrange, InvalidSurface };

/// Asynchronous audio contract; tokens identify playback completion/cancellation.
class AudioPort {
public:
    virtual ~AudioPort() = default;
    /// Queue or start a sound; false means playback is unavailable.
    virtual bool play(Sound sound, std::uint32_t token) = 0;
    /// Interrupt the current sound without blocking the controller.
    virtual void stop() = 0;
    /// Report completion for the specified play token.
    virtual bool finished(std::uint32_t token) const = 0;
    /// Return the source duration in milliseconds for controller timeout bounds.
    virtual std::uint32_t durationMs(Sound sound) const = 0;
};
} // namespace portal
