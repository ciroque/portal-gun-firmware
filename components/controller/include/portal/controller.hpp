#pragma once
#include "portal/audio_port.hpp"
#include "portal/debounce.hpp"
#include "portal/lighting.hpp"

namespace portal {
/// Visible phases of one prop interaction; Idle also hosts background animation.
enum class State { Idle, Spinup, Blast, Discharge, Decay, FailedShot, AwaitAudio };
/// Stable diagnostic label for logs and serial monitoring.
const char* stateName(State state);

/// Injected randomness keeps failed-shot selection deterministic in host tests.
class ShotRandom {
public:
    /// Allow a test or hardware source to be destroyed through this interface.
    virtual ~ShotRandom() = default;
    /// Return whether this trigger press selects the configured failed-shot event.
    virtual bool failedShot() = 0;
};
class Controller {
public:
    /// Compose the state machine from output, audio, random, and persisted settings ports.
    Controller(LightingPort& lights, AudioPort& audio, ShotRandom& random, Settings settings = {});
    /// Initialize black output and request the startup sound.
    void begin(std::uint32_t now);
    /// Deliver one debounced button press.
    void press(Button button, std::uint32_t now);
    /// Advance animation/audio state using a monotonic millisecond clock.
    void tick(std::uint32_t now);
    /// Current interaction state, exposed for diagnostics and tests.
    State state() const { return state_; }
    /// Current persistent settings.
    const Settings& settings() const { return settings_; }
    /// Current spin-up frame index; -1 when spin-up is inactive.
    int spinStep() const { return spin_step_; }
    /// Consume the settings-dirty flag after an accepted setting change.
    bool takeSettingsChanged();
private:
    /// Visual setting indicators that can replace one another while idle.
    enum class Feedback { None, Curve, Color };
    /// Store the frame and synchronously pass it to the lighting adapter.
    void present(const Frame& frame);
    /// Start a sound and record its completion token and timeout.
    void play(Sound sound, std::uint32_t now);
    /// Advance the idle animation when its frame deadline has elapsed.
    void idle(std::uint32_t now);
    /// Reset idle animation timing after a firing sequence.
    void resetIdle(std::uint32_t last_frame);
    /// Leave an effect only after both its visual and audio portions complete.
    void visualComplete(std::uint32_t now);
    LightingPort& lights_;
    AudioPort& audio_;
    ShotRandom& random_;
    Settings settings_;
    State state_ = State::Idle;
    Feedback feedback_ = Feedback::None;
    Frame frame_{};
    unsigned charge_ = 0;
    bool settings_changed_ = false, audio_pending_ = false;
    std::uint32_t last_frame_ = 0, phase_start_ = 0, spin_start_ = 0, spin_duration_ = 0;
    std::uint32_t feedback_start_ = 0, audio_start_ = 0, audio_timeout_ = 0, audio_id_ = 0;
    int idle_step_ = 0, idle_direction_ = 1, spin_step_ = 0, discharge_step_ = 0, decay_level_ = tuning::DecayInitial;
};
} // namespace portal
