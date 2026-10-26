#pragma once
#include "portal/settings.hpp"
#include <array>
#include <cstdint>

namespace portal {
struct Rgb { std::uint8_t r = 0, g = 0, b = 0; };
inline bool operator==(Rgb a, Rgb b) { return a.r == b.r && a.g == b.g && a.b == b.b; }
inline bool operator!=(Rgb a, Rgb b) { return !(a == b); }
using Frame = std::array<Rgb, 12>;
struct Palette { Rgb idle1, idle2, medium, hot, flash; };
Palette palette(PortalColor color);
Rgb blend(Rgb from, Rgb to, int step, int total);
Rgb scale(Rgb color, int level);
int wrapPixel(int pixel);
Frame fill(Rgb color);
Frame idleFrame(const Palette& colors, int step);
Frame spinFrame(const Palette& colors, int step);
Frame dischargeFrame(const Palette& colors, int step, int direction);
std::uint32_t spinDelay(Curve curve, int step);
std::uint32_t spinDuration(Curve curve);
unsigned chargeStage(std::uint32_t elapsed, std::uint32_t duration);

class LightingPort {
public:
    virtual ~LightingPort() = default;
    virtual void present(const Frame& frame, unsigned charge_stage) = 0;
};
} // namespace portal
