#include "doctest.h"
#include "portal/controller.hpp"
#include "portal/debounce.hpp"
#include "portal/settings.hpp"

#include <vector>

// Deterministic port doubles exercise timing, state transitions, and failure choices on the host.
using namespace portal;

struct TestLights : LightingPort {
    Frame frame{};
    unsigned charge = 0;
    void present(const Frame& value, unsigned stage) override { frame = value; charge = stage; }
};
struct TestAudio : AudioPort {
    bool available = true;
    bool done = false;
    int stops = 0;
    std::uint32_t id = 0;
    std::vector<Sound> played;
    bool play(Sound sound, std::uint32_t token) override {
        played.push_back(sound); id = token; done = false; return available;
    }
    void stop() override { ++stops; }
    bool finished(std::uint32_t token) const override { return done && token == id; }
    std::uint32_t durationMs(Sound) const override { return 1000; }
};
struct TestRandom : ShotRandom {
    bool fail = false;
    int calls = 0;
    bool failedShot() override { ++calls; return fail; }
};
struct Rig {
    TestLights lights;
    TestAudio audio;
    TestRandom random;
    Controller controller{lights, audio, random};
    Rig() { controller.begin(0); }
};

TEST_CASE("debounce accepts a press strictly after 30 stable milliseconds") {
    Debouncer button;
    CHECK_FALSE(button.poll(false, 10));
    CHECK_FALSE(button.poll(true, 12));
    CHECK_FALSE(button.poll(false, 14));
    CHECK_FALSE(button.poll(false, 44));
    CHECK(button.poll(false, 45));
    CHECK_FALSE(button.poll(false, 1000));
    CHECK_FALSE(button.poll(true, 1001));
    CHECK_FALSE(button.poll(true, 1032));
    CHECK_FALSE(button.poll(false, 1033));
    CHECK(button.poll(false, 1064));
}

TEST_CASE("debounce survives the 32-bit millisecond rollover") {
    Debouncer button;
    CHECK_FALSE(button.poll(false, 0xfffffff0u));
    CHECK_FALSE(button.poll(false, 14));
    CHECK(button.poll(false, 15));
}

TEST_CASE("settings serialization rejects corrupt and incompatible data") {
    Settings settings{PortalColor::Orange, Curve::Cubic, 1};
    auto bytes = encodeSettings(settings);
    Settings decoded;
    REQUIRE(decodeSettings(bytes.data(), bytes.size(), decoded));
    CHECK(decoded.color == PortalColor::Orange);
    CHECK(decoded.curve == Curve::Cubic);
    CHECK(decoded.direction == 1);
    for (unsigned index : {0u, 4u, 5u, 6u, 7u}) {
        auto corrupt = bytes;
        corrupt[index] = 0x80;
        CHECK_FALSE(decodeSettings(corrupt.data(), corrupt.size(), decoded));
    }
    CHECK_FALSE(decodeSettings(bytes.data(), bytes.size() - 1, decoded));
    CHECK_FALSE(decodeSettings(nullptr, 0, decoded));
    CHECK(Settings{}.direction == -1);
}

TEST_CASE("normal firing preserves frame deadlines and the exact blast boundary") {
    Rig rig;
    rig.controller.press(Button::Trigger, 100);
    CHECK(rig.audio.stops == 1);
    CHECK(rig.controller.state() == State::Spinup);
    rig.controller.tick(249);
    CHECK(rig.controller.spinStep() == 0);
    std::uint32_t now = 100;
    for (int step = 0; step < 48; ++step) {
        now += spinDelay(Curve::Linear, step);
        rig.controller.tick(now);
    }
    REQUIRE(rig.controller.state() == State::Blast);
    CHECK(rig.lights.charge == 3);
    for (auto pixel : rig.lights.frame) CHECK(pixel == Rgb{255, 255, 255});
    CHECK(rig.audio.played.back() == Sound::FireBlue);
    rig.controller.tick(now + 239);
    CHECK(rig.controller.state() == State::Blast);
    rig.controller.tick(now + 240);
    CHECK(rig.controller.state() == State::Discharge);
    CHECK(rig.lights.charge == 0);
    now += 241;
    for (int step = 0; step < 30; ++step, now += 18) rig.controller.tick(now);
    REQUIRE(rig.controller.state() == State::Decay);
    for (int step = 0; step < 22; ++step, now += 18) rig.controller.tick(now);
    CHECK(rig.controller.state() == State::AwaitAudio);
    // A failed audio worker cannot leave the controller busy indefinitely.
    rig.controller.tick(now + 2000);
    CHECK(rig.controller.state() == State::Idle);
}

TEST_CASE("charge thirds adapt to every selected curve and completion is phase-owned") {
    for (int curve = 0; curve < 5; ++curve) {
        const auto selected = static_cast<Curve>(curve);
        CHECK(chargeStage(0, spinDuration(selected)) == 0);
        CHECK(chargeStage((spinDuration(selected) + 2) / 3, spinDuration(selected)) == 1);
        CHECK(chargeStage((spinDuration(selected) * 2 + 2) / 3, spinDuration(selected)) == 2);
        CHECK(chargeStage(spinDuration(selected) + 100, spinDuration(selected)) == 2);
    }
    Rig rig;
    rig.controller.press(Button::Trigger, 100);
    rig.controller.tick(100000); // One late update must not catch up 48 frames.
    CHECK(rig.controller.state() == State::Spinup);
    CHECK(rig.controller.spinStep() == 1);
    CHECK(rig.lights.charge == 2);
}

TEST_CASE("settings feedback is interruptible and busy-state settings are ignored") {
    Rig rig;
    rig.controller.press(Button::Curve, 100);
    CHECK(rig.lights.frame[0] == Rgb{0, 255, 0});
    CHECK(rig.controller.takeSettingsChanged());
    rig.controller.press(Button::Color, 110);
    CHECK(rig.controller.settings().color == PortalColor::Orange);
    CHECK(rig.lights.frame[0] == palette(PortalColor::Orange).hot);
    rig.controller.press(Button::Trigger, 120);
    rig.controller.press(Button::Direction, 121);
    rig.controller.press(Button::Trigger, 122);
    CHECK(rig.controller.state() == State::Spinup);
    CHECK(rig.controller.settings().direction == -1);
    CHECK(rig.random.calls == 1);
    CHECK(rig.lights.frame[0] != Rgb{0, 255, 0});
}

TEST_CASE("failed shots flicker without blast and wait for their own audio token") {
    Rig rig;
    rig.random.fail = true;
    rig.controller.press(Button::Trigger, 100);
    CHECK(rig.controller.state() == State::FailedShot);
    CHECK(rig.audio.played.back() == Sound::InvalidSurface);
    CHECK(rig.lights.charge == 0);
    rig.controller.tick(300);
    CHECK(rig.controller.state() == State::AwaitAudio);
    rig.controller.press(Button::Trigger, 301);
    CHECK(rig.random.calls == 1);
    const auto shot_token = rig.audio.id;
    rig.audio.id = shot_token - 1; // Late startup completion must not release this shot.
    rig.audio.done = true;
    rig.controller.tick(302);
    CHECK(rig.controller.state() == State::AwaitAudio);
    rig.audio.id = shot_token;
    rig.controller.tick(303);
    CHECK(rig.controller.state() == State::Idle);
}

TEST_CASE("missing audio does not prevent completion of the failed-shot effect") {
    Rig rig;
    rig.audio.available = false;
    rig.random.fail = true;
    rig.controller.press(Button::Trigger, 100);
    rig.controller.tick(300);
    CHECK(rig.controller.state() == State::Idle);
}

TEST_CASE("curve feedback keeps its blink count and timing without blocking") {
    Rig rig;
    rig.controller.press(Button::Curve, 100); // Quadratic: two green blinks.
    rig.controller.tick(219);
    CHECK(rig.lights.frame[0] == Rgb{0, 255, 0});
    rig.controller.tick(220);
    CHECK(rig.lights.frame == Frame{});
    rig.controller.tick(340);
    CHECK(rig.lights.frame[0] == Rgb{0, 255, 0});
    rig.controller.tick(460);
    CHECK(rig.lights.frame == Frame{});
    rig.controller.tick(580);
    CHECK(rig.lights.frame == fill(palette(PortalColor::Blue).idle1));
    rig.controller.press(Button::Color, 600);
    rig.controller.tick(799);
    CHECK(rig.lights.frame == fill(palette(PortalColor::Orange).hot));
    rig.controller.tick(800);
    CHECK(rig.lights.frame == fill(palette(PortalColor::Orange).idle1));
}

TEST_CASE("firing deadlines survive a millisecond rollover") {
    Rig rig;
    std::uint32_t now = 0xffffffd0u;
    rig.controller.press(Button::Trigger, now);
    for (int step = 0; step < 48; ++step) {
        now += spinDelay(Curve::Linear, step);
        rig.controller.tick(now);
    }
    REQUIRE(rig.controller.state() == State::Blast);
    rig.controller.tick(now + 239);
    CHECK(rig.controller.state() == State::Blast);
    rig.controller.tick(now + 240);
    CHECK(rig.controller.state() == State::Discharge);
}
