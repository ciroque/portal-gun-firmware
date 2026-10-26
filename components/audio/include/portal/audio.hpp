#pragma once
#include "portal/audio_port.hpp"
#include "portal/wav.hpp"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <array>
#include <atomic>

namespace portal {
class I2sAudio : public AudioPort {
public:
    I2sAudio() = default;
    ~I2sAudio();
    I2sAudio(const I2sAudio&) = delete;
    I2sAudio& operator=(const I2sAudio&) = delete;
    bool init();
    bool play(Sound sound, std::uint32_t token) override;
    void stop() override;
    bool finished(std::uint32_t token) const override;
    std::uint32_t durationMs(Sound sound) const override;
private:
    struct Command { Sound sound; std::uint32_t token; bool stop; };
    static void taskEntry(void* self);
    void run();
    bool silence();
    bool recoverChannel();
    std::array<WavView, 4> assets_{};
    i2s_chan_handle_t channel_ = nullptr;
    QueueHandle_t commands_ = nullptr;
    TaskHandle_t task_ = nullptr;
    std::atomic<bool> ready_{false};
    std::atomic<std::uint32_t> completed_{0};
    bool channel_enabled_ = false;
};
} // namespace portal
