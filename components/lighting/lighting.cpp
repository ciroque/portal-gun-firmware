#include "portal/lighting.hpp"
#include <cmath>

namespace portal {
Palette palette(PortalColor color) {
    if (color == PortalColor::Blue)
        return {{140, 0, 255}, {0, 90, 255}, {20, 30, 160}, {70, 120, 255}, {160, 210, 255}};
    return {{255, 70, 0}, {255, 190, 0}, {180, 45, 0}, {255, 130, 0}, {255, 220, 80}};
}
Rgb blend(Rgb from, Rgb to, int step, int total) {
    auto channel = [=](int a, int b) {
        return static_cast<std::uint8_t>(a + (static_cast<std::int32_t>(b - a) * step) / total);
    };
    return {channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b)};
}
Rgb scale(Rgb color, int level) {
    return {static_cast<std::uint8_t>(color.r * level / 255),
        static_cast<std::uint8_t>(color.g * level / 255),
        static_cast<std::uint8_t>(color.b * level / 255)};
}
int wrapPixel(int pixel) { pixel %= 12; return pixel < 0 ? pixel + 12 : pixel; }
Frame fill(Rgb color) { Frame frame; frame.fill(color); return frame; }
Frame idleFrame(const Palette& colors, int step) { return fill(blend(colors.idle1, colors.idle2, step, 100)); }
Frame spinFrame(const Palette& colors, int step) {
    auto frame = fill(scale(colors.idle1, 40));
    const int head = step % 12;
    const int level = step * 205 / 47 + 50; // Arduino map() uses integer division.
    frame[head] = scale(colors.hot, level);
    frame[wrapPixel(head - 1)] = scale(colors.medium, level);
    frame[wrapPixel(head - 2)] = scale(colors.medium, level / 3);
    return frame;
}
Frame dischargeFrame(const Palette& colors, int step, int direction) {
    Frame frame{};
    const int head = wrapPixel(direction * step);
    frame[head] = colors.flash;
    frame[wrapPixel(head + direction)] = colors.hot;
    frame[wrapPixel(head - direction)] = colors.hot;
    frame[wrapPixel(head - 2 * direction)] = colors.medium;
    return frame;
}
std::uint32_t spinDelay(Curve curve, int step) {
    const float progress = static_cast<float>(step) / 47.0f;
    float value = progress;
    switch (curve) {
    case Curve::Linear: break;
    case Curve::Quadratic: value = progress * progress; break;
    case Curve::Cubic: value = progress * progress * progress; break;
    case Curve::Exponential: {
        const float numerator = static_cast<float>(std::exp(static_cast<double>(5.0f * progress)) - 1.0);
        const float denominator = static_cast<float>(std::exp(5.0) - 1.0);
        value = numerator / denominator;
        break;
    }
    case Curve::Smoothstep: value = progress * progress * (3.0f - 2.0f * progress); break;
    }
    return static_cast<std::uint32_t>(150 - 140.0f * value);
}
std::uint32_t spinDuration(Curve curve) {
    std::uint32_t total = 0;
    for (int step = 0; step < 48; ++step) total += spinDelay(curve, step);
    return total;
}
unsigned chargeStage(std::uint32_t elapsed, std::uint32_t duration) {
    if (!duration) return 0;
    const auto stage = static_cast<std::uint64_t>(elapsed) * 3 / duration;
    return stage > 2 ? 2 : static_cast<unsigned>(stage); // Third LED belongs to BLAST entry.
}
} // namespace portal
