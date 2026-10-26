#pragma once

// GPIO identifiers, not header positions. See ADR-0008 and board photos.
// Integers keep this board contract usable by the portable host tests.
namespace portal::pins {
inline constexpr int Trigger = 4;
inline constexpr int ColorButton = 5;
inline constexpr int CurveButton = 6;
inline constexpr int DirectionButton = 7;
inline constexpr int NeoPixelData = 8;
inline constexpr int ChargeLed1 = 9;
inline constexpr int ChargeLed2 = 10;
inline constexpr int ChargeLed3 = 11;
inline constexpr int MuzzleReserved = 12;
inline constexpr int I2sBclk = 15;
inline constexpr int I2sLrclk = 16;
inline constexpr int I2sData = 17;
}  // namespace portal::pins
