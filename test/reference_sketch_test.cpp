#include "doctest.h"
#include "portal/controller.hpp"
#include <math.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// Compile the unmodified sketch as an independent oracle. Only physical I/O,
// EEPROM and time are replaced; formulas, palettes and updates remain original.
constexpr bool HIGH = true, LOW = false;
constexpr int INPUT_PULLUP = 0;
static std::uint32_t reference_now = 0;
// Arduino clock stubs share one manually advanced time base for deterministic timelines.
unsigned long millis() { return reference_now; }
/// Advance reference time without waiting on the host process.
void delay(unsigned long duration) { reference_now += duration; }
/// Hardware setup is intentionally inert in the host oracle.
void pinMode(int, int) {}
/// Keep the pull-up inputs released unless a test explicitly changes the stub.
bool digitalRead(int) { return HIGH; }
/// Match Arduino's integer interpolation used by the original sketch.
long map(long value, long low, long high, long out_low, long out_high) {
    return (value - low) * (out_high - out_low) / (high - low) + out_low;
}
namespace reference {
// Declare sketch functions before including firmware.ino inside this namespace.
struct RGB;
void loadConfig(); bool isConfigValid(); void saveConfig(); void updateButtons();
void fire(); void togglePortalColor(); void toggleDischargeDirection(); void toggleAccelerationCurve();
void blinkCurveIndicator(int); void setPortalPalette(); void updateIdle(); void updateSpinup();
unsigned long calculateSpinDelay(int); void updateBlast(); void updateDischarge(); void updateDecay();
int wrapPixel(int); RGB blendColors(RGB, RGB, int, int); RGB scaleColor(RGB, int);
void fillRing(RGB); void fillRingNoShow(RGB);
#include "../firmware.ino"
}

namespace {
/// Compare every logical RGB pixel against one frame emitted by the sketch fake.
void compare(const portal::Frame& frame, const Adafruit_NeoPixel::Pixels& expected) {
    for (std::size_t i = 0; i < frame.size(); ++i) {
        CAPTURE(i);
        const auto color = frame[i];
        CHECK(((std::uint32_t{color.r} << 16) | (std::uint32_t{color.g} << 8) | color.b) == expected[i]);
    }
}
/// Reset all reference-sketch globals so each oracle case starts from idle.
void resetReference(int color, int curve = 0, int direction = -1) {
    reference_now = 0;
    reference::portalColor = static_cast<reference::PortalColor>(color);
    reference::accelerationCurve = static_cast<reference::AccelerationCurve>(curve);
    reference::dischargeDirection = direction;
    reference::animationState = reference::STATE_IDLE;
    reference::idleStep = 0; reference::idleDirection = 1;
    reference::lastFrameTime = 0; reference::stateStartTime = 0;
    reference::spinStep = 0; reference::dischargeStep = 0; reference::decayLevel = 220;
    reference::ring.clear(); reference::ring.shown.clear(); reference::setPortalPalette();
}
/// Capture production frame requests while preserving the controller's port interface.
struct OracleLights : portal::LightingPort {
    portal::Frame frame{};
    /// Retain the latest frame for comparison with the sketch renderer.
    void present(const portal::Frame& value, unsigned) override { frame = value; }
};
/// Model unavailable audio so reference-timeline tests stay independent of hardware.
struct SilentAudio : portal::AudioPort {
    /// Always report that no playback was started.
    bool play(portal::Sound, std::uint32_t) override { return false; }
    /// There is no active playback to cancel.
    void stop() override {}
    /// Unavailable playback is already complete from the controller's perspective.
    bool finished(std::uint32_t) const override { return true; }
    /// Unavailable assets have no duration.
    std::uint32_t durationMs(portal::Sound) const override { return 0; }
};
/// Select normal firing for comparisons against the original successful-shot path.
struct SuccessfulShot : portal::ShotRandom {
    /// Keep every reference timeline on the normal firing path.
    bool failedShot() override { return false; }
};
}

TEST_CASE("all curve delays match the original sketch") {
    for (int curve = 0; curve < 5; ++curve) {
        resetReference(0, curve);
        std::uint32_t duration = 0;
        for (int step = 0; step < 48; ++step) {
            CAPTURE(curve); CAPTURE(step);
            const auto expected = reference::calculateSpinDelay(step);
            CHECK(portal::spinDelay(static_cast<portal::Curve>(curve), step) == expected);
            duration += expected;
        }
        CHECK(portal::spinDuration(static_cast<portal::Curve>(curve)) == duration);
    }
}

TEST_CASE("every palette frame matches the original sketch renderers") {
    for (int color = 0; color < 2; ++color) {
        resetReference(color);
        const auto colors = portal::palette(static_cast<portal::PortalColor>(color));
        for (int step = 0; step <= 100; ++step) {
            reference_now += 20;
            reference::idleStep = step;
            reference::updateIdle();
            compare(portal::idleFrame(colors, step), reference::ring.pixels);
        }
        for (int step = 0; step < 48; ++step) {
            reference_now += 150;
            reference::spinStep = step;
            reference::ring.shown.clear();
            reference::updateSpinup();
            REQUIRE_FALSE(reference::ring.shown.empty());
            compare(portal::spinFrame(colors, step), reference::ring.shown.front());
        }
        for (int direction : {-1, 1}) {
            reference::dischargeDirection = direction;
            for (int step = 0; step < 30; ++step) {
                reference_now += 18;
                reference::dischargeStep = step;
                reference::updateDischarge();
                compare(portal::dischargeFrame(colors, step, direction), reference::ring.pixels);
            }
        }
        for (int level = 220; level > 0; level -= 10) {
            reference_now += 18;
            reference::decayLevel = level;
            reference::updateDecay();
            compare(portal::fill(portal::scale(colors.hot, level)), reference::ring.pixels);
        }
    }
}

TEST_CASE("complete firing timelines match the sketch even with late controller updates") {
    for (int curve = 0; curve < 5; ++curve) {
        for (int color = 0; color < 2; ++color) {
            for (int direction : {-1, 1}) {
                resetReference(color, curve, direction);
                OracleLights lights; SilentAudio audio; SuccessfulShot random;
                portal::Controller controller(lights, audio, random,
                    {static_cast<portal::PortalColor>(color), static_cast<portal::Curve>(curve),
                     static_cast<std::int8_t>(direction)});
                controller.begin(0);
                reference_now = 100;
                reference::fire(); controller.press(portal::Button::Trigger, reference_now);
                for (int update = 0; update < 2000; ++update) {
                    // Include missed deadlines without changing the reference's no-catch-up policy.
                    reference_now += update % 17 == 0 ? 37 : 7;
                    switch (reference::animationState) {
                    case reference::STATE_IDLE: reference::updateIdle(); break;
                    case reference::STATE_SPINUP: reference::updateSpinup(); break;
                    case reference::STATE_BLAST: reference::updateBlast(); break;
                    case reference::STATE_DISCHARGE: reference::updateDischarge(); break;
                    case reference::STATE_DECAY: reference::updateDecay(); break;
                    }
                    controller.tick(reference_now);
                    CAPTURE(curve); CAPTURE(color); CAPTURE(direction); CAPTURE(update);
                    CHECK(static_cast<int>(controller.state()) == static_cast<int>(reference::animationState));
                    compare(lights.frame, reference::ring.pixels);
                    reference::ring.shown.clear();
                    if (controller.state() == portal::State::Idle) break;
                    REQUIRE(update < 1999);
                }
            }
        }
    }
}
