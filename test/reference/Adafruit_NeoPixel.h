#pragma once
#include <array>
#include <cstdint>
#include <vector>
constexpr int NEO_GRB = 0, NEO_KHZ800 = 0;
class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(int, int, int) {}
    using Pixels = std::array<std::uint32_t, 12>;
    Pixels pixels{};
    std::vector<Pixels> shown;
    void begin() {}
    void clear() { pixels.fill(0); }
    void setBrightness(int) {} // Reference always uses 255.
    std::uint32_t Color(int r, int g, int b) { return (r << 16) | (g << 8) | b; }
    void setPixelColor(int index, std::uint32_t color) { pixels.at(index) = color; }
    void show() { shown.push_back(pixels); }
};
