#include "portal/ring.hpp"
#include "portal/pins.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include <cstdlib>
#include <cstring>

namespace {
constexpr std::array<int, 3> charge_pins{portal::pins::ChargeLed1, portal::pins::ChargeLed2, portal::pins::ChargeLed3};
constexpr std::uint32_t RmtResolutionHz = 20000000;

struct LedEncoder {
    rmt_encoder_t base{};
    rmt_encoder_handle_t bytes = nullptr;
    rmt_encoder_handle_t copy = nullptr;
    int stage = 0;
    rmt_symbol_word_t reset{};
};

// RMT encodes the GRB byte stream first, then appends one low reset/latch symbol.
size_t encodeLed(rmt_encoder_t* base, rmt_channel_handle_t channel, const void* data,
                 size_t size, rmt_encode_state_t* result) {
    auto* encoder = reinterpret_cast<LedEncoder*>(base);
    rmt_encode_state_t session = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t symbols = 0;
    switch (encoder->stage) {
    case 0:
        symbols += encoder->bytes->encode(encoder->bytes, channel, data, size, &session);
        if (session & RMT_ENCODING_COMPLETE) encoder->stage = 1;
        if (session & RMT_ENCODING_MEM_FULL) {
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
            goto done;
        }
        [[fallthrough]];
    case 1:
        symbols += encoder->copy->encode(encoder->copy, channel, &encoder->reset,
                                         sizeof(encoder->reset), &session);
        if (session & RMT_ENCODING_COMPLETE) {
            encoder->stage = 0;
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_COMPLETE);
        }
        if (session & RMT_ENCODING_MEM_FULL)
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
        break;
    }
done:
    *result = state;
    return symbols;
}

esp_err_t resetLed(rmt_encoder_t* base) {
    auto* encoder = reinterpret_cast<LedEncoder*>(base);
    rmt_encoder_reset(encoder->bytes);
    rmt_encoder_reset(encoder->copy);
    encoder->stage = 0;
    return ESP_OK;
}

esp_err_t deleteLed(rmt_encoder_t* base) {
    auto* encoder = reinterpret_cast<LedEncoder*>(base);
    if (encoder->bytes) rmt_del_encoder(encoder->bytes);
    if (encoder->copy) rmt_del_encoder(encoder->copy);
    std::free(encoder);
    return ESP_OK;
}

esp_err_t newLedEncoder(rmt_encoder_handle_t* result) {
    auto* encoder = static_cast<LedEncoder*>(rmt_alloc_encoder_mem(sizeof(LedEncoder)));
    if (!encoder) return ESP_ERR_NO_MEM;
    std::memset(encoder, 0, sizeof(*encoder));
    encoder->base.encode = encodeLed;
    encoder->base.reset = resetLed;
    encoder->base.del = deleteLed;

    rmt_bytes_encoder_config_t bytes_config{};
    bytes_config.bit0.level0 = 1; bytes_config.bit0.duration0 = 7;
    bytes_config.bit0.level1 = 0; bytes_config.bit0.duration1 = 18;
    bytes_config.bit1.level0 = 1; bytes_config.bit1.duration0 = 16;
    bytes_config.bit1.level1 = 0; bytes_config.bit1.duration1 = 9;
    bytes_config.flags.msb_first = 1;
    auto error = rmt_new_bytes_encoder(&bytes_config, &encoder->bytes);
    if (error == ESP_OK) {
        rmt_copy_encoder_config_t copy_config{};
        error = rmt_new_copy_encoder(&copy_config, &encoder->copy);
    }
    if (error != ESP_OK) {
        deleteLed(&encoder->base);
        return error;
    }
    encoder->reset.level0 = 0; encoder->reset.duration0 = 3000;
    encoder->reset.level1 = 0; encoder->reset.duration1 = 3000; // 300 us low latch at 20 MHz.
    *result = &encoder->base;
    return ESP_OK;
}
} // namespace

namespace portal {
RingLights::~RingLights() {
    if (channel_) { rmt_disable(channel_); rmt_del_channel(channel_); }
    if (encoder_) rmt_del_encoder(encoder_);
}
bool RingLights::init() {
    gpio_config_t charge_config{};
    for (auto pin : charge_pins) charge_config.pin_bit_mask |= std::uint64_t{1} << pin;
    charge_config.mode = GPIO_MODE_OUTPUT;
    charge_ready_ = gpio_config(&charge_config) == ESP_OK;
    if (charge_ready_) for (auto pin : charge_pins) gpio_set_level(static_cast<gpio_num_t>(pin), 0);

    rmt_tx_channel_config_t config{};
    config.gpio_num = static_cast<gpio_num_t>(pins::NeoPixelData);
    config.clk_src = RMT_CLK_SRC_DEFAULT;
    config.resolution_hz = RmtResolutionHz;
    config.mem_block_symbols = 64;
    config.trans_queue_depth = 1;
    auto error = rmt_new_tx_channel(&config, &channel_);
    if (error == ESP_OK) error = newLedEncoder(&encoder_);
    if (error == ESP_OK) error = rmt_enable(channel_);
    ring_ready_ = error == ESP_OK;
    if (!ring_ready_) {
        ESP_LOGE("lighting", "RMT initialization failed: %s", esp_err_to_name(error));
        if (encoder_) { rmt_del_encoder(encoder_); encoder_ = nullptr; }
        if (channel_) { rmt_del_channel(channel_); channel_ = nullptr; }
    }
    if (!charge_ready_) ESP_LOGE("lighting", "Charge LED GPIO initialization failed");
    present(Frame{}, 0);
    return ring_ready_ && charge_ready_;
}
void RingLights::present(const Frame& frame, unsigned charge_stage) {
    if (charge_ready_) for (unsigned i = 0; i < charge_pins.size(); ++i)
        gpio_set_level(static_cast<gpio_num_t>(charge_pins[i]), i < charge_stage);
    if (!ring_ready_) return;
    // Never overwrite the payload until the RMT encoder has finished consuming it.
    if (tx_pending_) {
        const auto completed = rmt_tx_wait_all_done(channel_, 5);
        if (completed != ESP_OK) {
            ESP_LOGW("lighting", "Previous RMT frame still pending: %s", esp_err_to_name(completed));
            return;
        }
        tx_pending_ = false;
        previous_ = pending_frame_;
        have_frame_ = true;
    }
    if (have_frame_ && frame == previous_) return;
    std::size_t index = 0;
    for (auto color : frame) {
        bytes_[index++] = color.g;
        bytes_[index++] = color.r;
        bytes_[index++] = color.b;
    }
    rmt_transmit_config_t transmit{};
    auto error = rmt_transmit(channel_, encoder_, bytes_.data(), bytes_.size(), &transmit);
    if (error != ESP_OK) {
        ESP_LOGW("lighting", "RMT transmit failed; will retry next frame: %s", esp_err_to_name(error));
        return;
    }
    pending_frame_ = frame;
    tx_pending_ = true;
    error = rmt_tx_wait_all_done(channel_, 5);
    if (error != ESP_OK) {
        ESP_LOGW("lighting", "RMT frame pending; will retry after completion: %s", esp_err_to_name(error));
        return;
    }
    tx_pending_ = false;
    previous_ = frame;
    have_frame_ = true;
}
} // namespace portal
