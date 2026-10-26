#include "portal/ring.hpp"
#include "portal/pins.hpp"
#include "driver/gpio.h"
#include "esp_log.h"

namespace portal {
namespace {
constexpr std::array<int, 3> charge_pins{pins::ChargeLed1, pins::ChargeLed2, pins::ChargeLed3};
}
RingLights::~RingLights() {
    if (channel_) { rmt_disable(channel_); rmt_del_channel(channel_); }
    if (encoder_) rmt_del_encoder(encoder_);
}
bool RingLights::init() {
    gpio_config_t charge_config{};
    for (auto pin : charge_pins) {
        gpio_set_level(static_cast<gpio_num_t>(pin), 0);
        charge_config.pin_bit_mask |= std::uint64_t{1} << pin;
    }
    charge_config.mode = GPIO_MODE_OUTPUT;
    charge_ready_ = gpio_config(&charge_config) == ESP_OK;
    rmt_tx_channel_config_t config{};
    config.gpio_num = static_cast<gpio_num_t>(pins::NeoPixelData);
    config.clk_src = RMT_CLK_SRC_DEFAULT;
    config.resolution_hz = 20000000;
    config.mem_block_symbols = 64;
    config.trans_queue_depth = 1;
    auto error = rmt_new_tx_channel(&config, &channel_);
    rmt_copy_encoder_config_t encoder_config{};
    if (error == ESP_OK) error = rmt_new_copy_encoder(&encoder_config, &encoder_);
    if (error == ESP_OK) error = rmt_enable(channel_);
    ring_ready_ = error == ESP_OK;
    if (!ring_ready_) ESP_LOGE("lighting", "RMT initialization failed: %s", esp_err_to_name(error));
    if (!charge_ready_) ESP_LOGE("lighting", "Charge LED GPIO initialization failed");
    present(Frame{}, 0);
    return ring_ready_ && charge_ready_;
}
void RingLights::present(const Frame& frame, unsigned charge_stage) {
    if (charge_ready_) for (unsigned i = 0; i < charge_pins.size(); ++i)
        gpio_set_level(static_cast<gpio_num_t>(charge_pins[i]), i < charge_stage);
    if (!ring_ready_ || (have_frame_ && frame == previous_)) return;
    std::size_t index = 0;
    for (auto color : frame) {
        // The ring is GRB, 800 kHz. Hardware RMT continues alongside I2S DMA.
        for (auto byte : {color.g, color.r, color.b}) for (int bit = 7; bit >= 0; --bit) {
            const bool one = (byte & (1u << bit)) != 0;
            auto& symbol = symbols_[index++];
            symbol.level0 = 1; symbol.duration0 = one ? 16 : 7;
            symbol.level1 = 0; symbol.duration1 = one ? 9 : 18;
        }
    }
    auto& reset = symbols_[index];
    reset.level0 = 0; reset.duration0 = 3000;
    reset.level1 = 0; reset.duration1 = 3000; // 300 us latch covers newer pixel revisions too.
    rmt_transmit_config_t transmit{};
    auto error = rmt_transmit(channel_, encoder_, symbols_.data(), sizeof(symbols_), &transmit);
    if (error == ESP_OK) error = rmt_tx_wait_all_done(channel_, 5);
    if (error != ESP_OK) {
        ESP_LOGE("lighting", "RMT output disabled after failure: %s", esp_err_to_name(error));
        ring_ready_ = false; // Keep the in-flight buffer intact; do not rewrite after a timeout.
        return;
    }
    previous_ = frame; have_frame_ = true;
}
} // namespace portal
