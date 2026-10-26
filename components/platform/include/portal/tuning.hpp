#pragma once
#include <cstddef>
#include <cstdint>

namespace portal::tuning {
// Reference effect values from firmware.ino; changes need explicit behavioral review.
inline constexpr std::size_t Pixels = 12;
inline constexpr int IdleSteps = 100, SpinSteps = 48, DischargeSteps = 30;
inline constexpr int DecayInitial = 220, DecayDecrement = 10;
inline constexpr std::uint32_t IdleFrameMs = 20, SpinStartMs = 150, SpinEndMs = 10;
inline constexpr std::uint32_t BlastMs = 240, DischargeFrameMs = 18, DecayFrameMs = 18;
inline constexpr std::uint32_t DebounceMs = 30, CurveOnMs = 120, CurveOffMs = 120;
inline constexpr float ExponentialStrength = 5.0f;
// Accepted additions; failed-shot flicker and audio amplitude are bench-tuning values.
inline constexpr unsigned ChargeLeds = 3;
inline constexpr std::uint32_t ColorPulseMs = 200, FailedFlickerMs = 200, FailedHalfPeriodMs = 50;
inline constexpr std::uint32_t AudioTimeoutMarginMs = 250, MaxAudioDurationMs = 10000;
inline constexpr std::uint16_t AudioVolumeQ15 = 8192;
} // namespace portal::tuning
