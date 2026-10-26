#pragma once
#include "portal/lighting.hpp"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

namespace portal {
class RingLights : public LightingPort {
public:
    RingLights() = default;
    ~RingLights();
    RingLights(const RingLights&) = delete;
    RingLights& operator=(const RingLights&) = delete;
    bool init();
    void present(const Frame& frame, unsigned charge_stage) override;
private:
    rmt_channel_handle_t channel_ = nullptr;
    rmt_encoder_handle_t encoder_ = nullptr;
    std::array<rmt_symbol_word_t, tuning::Pixels * 24 + 1> symbols_{};
    Frame previous_{};
    bool ring_ready_ = false, charge_ready_ = false, have_frame_ = false;
};
} // namespace portal
