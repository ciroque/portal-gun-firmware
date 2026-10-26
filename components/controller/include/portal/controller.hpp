#pragma once
#include "portal/audio_port.hpp"
#include "portal/debounce.hpp"
#include "portal/lighting.hpp"

namespace portal {
enum class State { Idle, Spinup, Blast, Discharge, Decay, FailedShot, AwaitAudio };
const char* stateName(State state);
class ShotRandom {
public:
    virtual ~ShotRandom() = default;
    virtual bool failedShot() = 0;
};
class Controller {
public:
    Controller(LightingPort& lights, AudioPort& audio, ShotRandom& random, Settings settings = {});
    void begin(std::uint32_t now);
    void press(Button button, std::uint32_t now);
    void tick(std::uint32_t now);
    State state() const { return state_; }
    const Settings& settings() const { return settings_; }
    int spinStep() const { return spin_step_; }
    bool takeSettingsChanged();
private:
    enum class Feedback { None, Curve, Color };
    void present(const Frame& frame);
    void play(Sound sound, std::uint32_t now);
    void idle(std::uint32_t now);
    void resetIdle(std::uint32_t last_frame);
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
