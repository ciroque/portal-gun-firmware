#pragma once
// Minimal host fake: record rendered pixels and show() snapshots for the sketch oracle.
#include <array>
#include <cstdint>
#include <vector>
constexpr int NEO_GRB = 0, NEO_KHZ800 = 0;
/// Minimal Adafruit_NeoPixel surface consumed by the unmodified sketch.
class Adafruit_NeoPixel {
public:
    /// Match the sketch's constructor without allocating or touching hardware.
    Adafruit_NeoPixel(int, int, int) {}
    using Pixels = std::array<std::uint32_t, 12>;
    Pixels pixels{};
    std::vector<Pixels> shown;
    /// Preserve the Arduino API used by the reference sketch.
    void begin() {}
    /// Clear the pending pixel buffer.
    void clear() { pixels.fill(0); }
    /// The reference sketch sets full brightness, so no scaling is needed here.
    void setBrightness(int) {} // Reference always uses 255.
    /// Pack RGB channels into the value format expected by the fake pixel buffer.
    std::uint32_t Color(int r, int g, int b) { return (r << 16) | (g << 8) | b; }
    /// Store one pixel in logical ring order.
    void setPixelColor(int index, std::uint32_t color) { pixels.at(index) = color; }
    /// Record an immutable snapshot for comparison with production frames.
    void show() { shown.push_back(pixels); }
};
