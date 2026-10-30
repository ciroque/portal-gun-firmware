#include "portal/controller.hpp"
#include <algorithm>

// Own behavior and timing here; hardware adapters are injected through the ports.
namespace portal {
const char* stateName(State state) {
    switch (state) {
    case State::Idle: return "IDLE";
    case State::Spinup: return "SPINUP";
    case State::Blast: return "BLAST";
    case State::Discharge: return "DISCHARGE";
    case State::Decay: return "DECAY";
    case State::FailedShot: return "FAILED_SHOT";
    case State::AwaitAudio: return "AWAIT_AUDIO";
    }
    return "UNKNOWN";
}
Controller::Controller(LightingPort& lights, AudioPort& audio, ShotRandom& random, Settings settings)
    : lights_(lights), audio_(audio), random_(random), settings_(validSettings(settings) ? settings : Settings{}) {}
void Controller::begin(std::uint32_t now) { present(Frame{}); play(Sound::Startup, now); }
void Controller::present(const Frame& frame) { frame_ = frame; lights_.present(frame_, charge_); }
void Controller::play(Sound sound, std::uint32_t now) {
    if (++audio_id_ == 0) ++audio_id_;
    audio_pending_ = audio_.play(sound, audio_id_);
    audio_start_ = now;
    audio_timeout_ = std::min(audio_.durationMs(sound), tuning::MaxAudioDurationMs) + tuning::AudioTimeoutMarginMs;
}
void Controller::resetIdle(std::uint32_t last_frame) {
    idle_step_ = 0; idle_direction_ = 1; last_frame_ = last_frame;
}
bool Controller::takeSettingsChanged() {
    const bool changed = settings_changed_; settings_changed_ = false; return changed;
}
void Controller::press(Button button, std::uint32_t now) {
    if (state_ != State::Idle) return;
    if (button == Button::Trigger) {
        const bool interrupted = feedback_ != Feedback::None;
        feedback_ = Feedback::None;
        audio_.stop(); audio_pending_ = false;
        charge_ = 0;
        if (random_.failedShot()) {
            state_ = State::FailedShot; phase_start_ = now;
            present(fill(palette(settings_.color).hot));
            play(Sound::InvalidSurface, now);
        } else {
            state_ = State::Spinup; spin_step_ = 0; last_frame_ = now; spin_start_ = now;
            spin_duration_ = spinDuration(settings_.curve);
            if (interrupted) present(fill(palette(settings_.color).idle1));
        }
        return;
    }
    settings_changed_ = true;
    if (button == Button::Direction) { settings_.direction *= -1; return; }
    feedback_start_ = now;
    if (button == Button::Color) {
        settings_.color = settings_.color == PortalColor::Blue ? PortalColor::Orange : PortalColor::Blue;
        feedback_ = Feedback::Color;
        present(fill(palette(settings_.color).hot));
    } else {
        settings_.curve = static_cast<Curve>((static_cast<unsigned>(settings_.curve) + 1) % 5);
        feedback_ = Feedback::Curve;
        Frame indicator{}; indicator[0] = {0, 255, 0}; present(indicator);
    }
}
void Controller::idle(std::uint32_t now) {
    if (now - last_frame_ < tuning::IdleFrameMs) return;
    last_frame_ = now;
    present(idleFrame(palette(settings_.color), idle_step_));
    idle_step_ += idle_direction_;
    if (idle_step_ >= tuning::IdleSteps) { idle_step_ = tuning::IdleSteps; idle_direction_ = -1; }
    if (idle_step_ <= 0) { idle_step_ = 0; idle_direction_ = 1; }
}
void Controller::visualComplete(std::uint32_t now) {
    state_ = audio_pending_ ? State::AwaitAudio : State::Idle;
    resetIdle(0);
    if (state_ == State::AwaitAudio) idle(now);
}
void Controller::tick(std::uint32_t now) {
    if (audio_pending_) {
        if (audio_.finished(audio_id_)) audio_pending_ = false;
        else if (now - audio_start_ >= audio_timeout_) { audio_.stop(); audio_pending_ = false; }
    }
    const auto colors = palette(settings_.color);
    switch (state_) {
    case State::Idle:
        if (feedback_ != Feedback::None) {
            const auto elapsed = now - feedback_start_;
            constexpr auto period = tuning::CurveOnMs + tuning::CurveOffMs;
            const auto duration = feedback_ == Feedback::Color ? tuning::ColorPulseMs :
                period * (static_cast<unsigned>(settings_.curve) + 1);
            if (elapsed >= duration) {
                feedback_ = Feedback::None; resetIdle(now); present(fill(colors.idle1));
            } else if (feedback_ == Feedback::Curve) {
                Frame indicator{};
                if (elapsed % period < tuning::CurveOnMs) indicator[0] = {0, 255, 0};
                if (indicator != frame_) present(indicator);
            }
        } else idle(now);
        break;
    case State::Spinup: {
        const auto stage = chargeStage(now - spin_start_, spin_duration_);
        if (stage != charge_) { charge_ = stage; present(frame_); }
        if (now - last_frame_ < spinDelay(settings_.curve, spin_step_)) break;
        last_frame_ = now;
        present(spinFrame(colors, spin_step_));
        if (++spin_step_ == tuning::SpinSteps) {
            state_ = State::Blast; phase_start_ = now; charge_ = tuning::ChargeLeds;
            present(fill({255, 255, 255}));
            play(settings_.color == PortalColor::Blue ? Sound::FireBlue : Sound::FireOrange, now);
        }
        break;
    }
    case State::Blast:
        if (now - phase_start_ >= tuning::BlastMs) {
            state_ = State::Discharge; discharge_step_ = 0; last_frame_ = 0;
            charge_ = 0; present(frame_);
        }
        break;
    case State::Discharge:
        if (now - last_frame_ >= tuning::DischargeFrameMs) {
            last_frame_ = now; present(dischargeFrame(colors, discharge_step_, settings_.direction));
            if (++discharge_step_ == tuning::DischargeSteps) { state_ = State::Decay; decay_level_ = tuning::DecayInitial; last_frame_ = 0; }
        }
        break;
    case State::Decay:
        if (now - last_frame_ >= tuning::DecayFrameMs) {
            last_frame_ = now; present(fill(scale(colors.hot, decay_level_)));
            decay_level_ -= tuning::DecayDecrement;
            if (decay_level_ <= 0) visualComplete(now);
        }
        break;
    case State::FailedShot: {
        const auto elapsed = now - phase_start_;
        if (elapsed >= tuning::FailedFlickerMs) visualComplete(now);
        else {
            const auto flicker = fill((elapsed / tuning::FailedHalfPeriodMs) % 2 == 0 ? colors.hot : Rgb{});
            if (flicker != frame_) present(flicker);
        }
        break;
    }
    case State::AwaitAudio:
        if (!audio_pending_) state_ = State::Idle;
        idle(now);
        break;
    }
}
} // namespace portal
