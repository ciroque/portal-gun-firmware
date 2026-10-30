#pragma once
#include "portal/lighting.hpp"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

namespace portal {
/// ESP-IDF RMT/GPIO implementation of the lighting output boundary.
class RingLights : public LightingPort {
public:
    RingLights() = default;
    ~RingLights();
    RingLights(const RingLights&) = delete;
    RingLights& operator=(const RingLights&) = delete;
    /// Configure the WS2812 transmitter and charge indicator outputs.
    bool init();
    /// Send changed frames and update charge outputs; failed RMT frames are retried later.
    void present(const Frame& frame, unsigned charge_stage) override;
private:
    rmt_channel_handle_t channel_ = nullptr;
    rmt_encoder_handle_t encoder_ = nullptr;
    std::array<std::uint8_t, tuning::Pixels * 3> bytes_{};
    Frame previous_{}, pending_frame_{};
    bool ring_ready_ = false, charge_ready_ = false, have_frame_ = false, tx_pending_ = false;
};
} // namespace portal
