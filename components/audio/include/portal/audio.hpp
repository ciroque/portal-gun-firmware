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
/// Asynchronous I2S playback backend for the embedded mono WAV assets.
class I2sAudio : public AudioPort {
public:
    /// Construct an uninitialized adapter; call init() before submitting sounds.
    I2sAudio() = default;
    /// Stop the worker and release I2S, queue, and task resources.
    ~I2sAudio();
    /// A running audio worker owns the underlying handles, so instances cannot be copied.
    I2sAudio(const I2sAudio&) = delete;
    /// Audio handles and queues are not transferable between instances.
    I2sAudio& operator=(const I2sAudio&) = delete;
    /// Configure I2S and start the dedicated playback worker.
    bool init();
    /// Request one sound under a token; returns false when audio is unavailable.
    bool play(Sound sound, std::uint32_t token) override;
    /// Request playback cancellation through the worker's command mailbox.
    void stop() override;
    /// Check whether this token has completed or audio has failed safely.
    bool finished(std::uint32_t token) const override;
    /// Return the validated WAV duration, or zero for an invalid/missing asset.
    std::uint32_t durationMs(Sound sound) const override;
private:
    /// One mailbox entry; stop commands supersede pending play requests.
    struct Command { Sound sound; std::uint32_t token; bool stop; };
    /// FreeRTOS trampoline that dispatches to run().
    static void taskEntry(void* self);
    /// Own the I2S channel and process playback commands until task deletion.
    void run();
    /// Replace queued audio with a silent DMA ring.
    bool silence();
    /// Re-enable the channel after a transient write or flush error.
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
