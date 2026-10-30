#pragma once
#include "portal/settings.hpp"
#include "portal/tuning.hpp"
#include <array>
#include <cstdint>

namespace portal {
/// One 8-bit-per-channel pixel value in logical RGB order.
struct Rgb { std::uint8_t r = 0, g = 0, b = 0; };
/// Compare logical pixel channels without converting to hardware byte order.
inline bool operator==(Rgb a, Rgb b) { return a.r == b.r && a.g == b.g && a.b == b.b; }
/// Negated channel comparison for C++17 frame equality.
inline bool operator!=(Rgb a, Rgb b) { return !(a == b); }
/// Full logical image for one update of the fixed-size ring.
using Frame = std::array<Rgb, tuning::Pixels>;
/// Effect colors for the selected portal palette.
struct Palette { Rgb idle1, idle2, medium, hot, flash; };
/// Return the original sketch's palette for the selected portal color.
Palette palette(PortalColor color);
/// Interpolate channels using the integer arithmetic of the reference sketch.
Rgb blend(Rgb from, Rgb to, int step, int total);
/// Scale channel intensity by an 8-bit level.
Rgb scale(Rgb color, int level);
/// Wrap a logical pixel index into the physical ring range.
int wrapPixel(int pixel);
/// Create a frame filled with one color.
Frame fill(Rgb color);
/// Render one idle, spin-up, or discharge frame from the reference effect.
Frame idleFrame(const Palette& colors, int step);
/// Render a spin-up frame with its moving head and trail.
Frame spinFrame(const Palette& colors, int step);
/// Render one discharge frame in the configured ring direction.
Frame dischargeFrame(const Palette& colors, int step, int direction);
/// Return the delay before a spin-up frame for the selected motion curve.
std::uint32_t spinDelay(Curve curve, int step);
/// Sum all frame delays for the selected curve.
std::uint32_t spinDuration(Curve curve);
/// Compute the number of charge indicators lit at elapsed spin time.
unsigned chargeStage(std::uint32_t elapsed, std::uint32_t duration);

/// Hardware boundary for pixel frames and the three charge indicators.
class LightingPort {
public:
    /// Allow adapters to be destroyed through this interface.
    virtual ~LightingPort() = default;
    /// Present the frame and update the independent charge indicators.
    virtual void present(const Frame& frame, unsigned charge_stage) = 0;
};
} // namespace portal
