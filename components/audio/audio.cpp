#include "portal/audio.hpp"
#include "portal/pins.hpp"
#include "portal/tuning.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <algorithm>

namespace {
extern const std::uint8_t startup_start[] asm("_binary_portalgun_powerup1_wav_start");
extern const std::uint8_t startup_end[] asm("_binary_portalgun_powerup1_wav_end");
extern const std::uint8_t blue_start[] asm("_binary_portalgun_shoot_blue1_wav_start");
extern const std::uint8_t blue_end[] asm("_binary_portalgun_shoot_blue1_wav_end");
extern const std::uint8_t orange_start[] asm("_binary_portalgun_shoot_red1_wav_start");
extern const std::uint8_t orange_end[] asm("_binary_portalgun_shoot_red1_wav_end");
extern const std::uint8_t invalid_start[] asm("_binary_portal_invalid_surface3_wav_start");
extern const std::uint8_t invalid_end[] asm("_binary_portal_invalid_surface3_wav_end");
constexpr const char* names[] = {"startup", "fire_blue", "fire_orange", "invalid_surface"};
std::uint32_t nowMs() { return static_cast<std::uint32_t>(esp_timer_get_time() / 1000); }
}

namespace portal {
I2sAudio::~I2sAudio() {
    ready_.store(false);
    if (task_) vTaskDelete(task_);
    if (channel_) { i2s_channel_disable(channel_); i2s_del_channel(channel_); }
    if (commands_) vQueueDelete(commands_);
}
bool I2sAudio::init() {
    const std::array<const std::uint8_t*, 4> starts{startup_start, blue_start, orange_start, invalid_start};
    const std::array<const std::uint8_t*, 4> ends{startup_end, blue_end, orange_end, invalid_end};
    for (unsigned i = 0; i < assets_.size(); ++i) {
        const auto size = reinterpret_cast<std::uintptr_t>(ends[i]) - reinterpret_cast<std::uintptr_t>(starts[i]);
        const auto wav = parseWav(starts[i], size);
        if (wav) assets_[i] = *wav;
        else ESP_LOGW("audio", "Invalid/missing asset: %s", names[i]);
    }
    i2s_chan_config_t config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    config.dma_desc_num = 4;
    config.dma_frame_num = 64;
    config.auto_clear_after_cb = true; // Underrun/idle produces silence, never a repeated last buffer.
    auto error = i2s_new_channel(&config, &channel_, nullptr);
    i2s_std_config_t standard{};
    standard.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100);
    standard.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    standard.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    standard.gpio_cfg.bclk = static_cast<gpio_num_t>(pins::I2sBclk);
    standard.gpio_cfg.ws = static_cast<gpio_num_t>(pins::I2sLrclk);
    standard.gpio_cfg.dout = static_cast<gpio_num_t>(pins::I2sData);
    standard.gpio_cfg.din = I2S_GPIO_UNUSED;
    if (error == ESP_OK) error = i2s_channel_init_std_mode(channel_, &standard);
    if (error == ESP_OK) error = i2s_channel_enable(channel_);
    if (error != ESP_OK) {
        ESP_LOGW("audio", "I2S unavailable: %s; controller will continue silently", esp_err_to_name(error));
        return false;
    }
    commands_ = xQueueCreate(1, sizeof(Command));
    if (!commands_ || xTaskCreate(taskEntry, "portal_audio", 4096, this, 5, &task_) != pdPASS) {
        ESP_LOGW("audio", "Playback task unavailable; controller will continue silently");
        return false;
    }
    ready_.store(true);
    return true;
}
bool I2sAudio::play(Sound sound, std::uint32_t token) {
    const auto index = static_cast<unsigned>(sound);
    if (!ready_.load() || index >= assets_.size() || !assets_[index].samples) return false;
    const Command command{sound, token, false};
    return xQueueOverwrite(commands_, &command) == pdPASS;
}
void I2sAudio::stop() {
    if (!ready_.load()) return;
    const Command command{Sound::Startup, 0, true};
    xQueueOverwrite(commands_, &command);
}
bool I2sAudio::finished(std::uint32_t token) const {
    return !ready_.load() || completed_.load() == token;
}
std::uint32_t I2sAudio::durationMs(Sound sound) const {
    const auto index = static_cast<unsigned>(sound);
    return index < assets_.size() ? assets_[index].durationMs() : 0;
}
void I2sAudio::taskEntry(void* self) { static_cast<I2sAudio*>(self)->run(); }
bool I2sAudio::silence() {
    // Flush one whole DMA ring on replacement. At 44.1 kHz this is ~5.8 ms;
    // existing buffered samples may still be audible briefly after cancellation.
    const std::array<std::int16_t, 4 * 64 * 2> zeros{};
    std::size_t written = 0;
    const auto error = i2s_channel_write(channel_, zeros.data(), sizeof(zeros), &written, 20);
    return error == ESP_OK && written == sizeof(zeros);
}
void I2sAudio::run() {
    Command current{Sound::Startup, 0, true};
    std::size_t position = 0;
    bool active = false, draining = false;
    std::uint32_t drain_start = 0;
    std::array<std::int16_t, 64 * 2> output{};
    for (;;) {
        Command command{};
        const auto wait = active ? TickType_t{0} : draining ? TickType_t{1} : portMAX_DELAY;
        if (xQueueReceive(commands_, &command, wait) == pdTRUE) {
            if (active || draining) completed_.store(current.token);
            active = false; draining = false;
            if (!silence()) {
                ready_.store(false);
                completed_.store(command.token);
                ESP_LOGW("audio", "DMA flush failed; playback disabled");
                continue;
            }
            if (command.stop || !ready_.load()) continue;
            current = command; position = 0; active = true;
            ESP_LOGI("audio", "play %s", names[static_cast<unsigned>(current.sound)]);
        }
        if (draining && nowMs() - drain_start >= 8) {
            completed_.store(current.token); draining = false;
        }
        if (!active) continue;
        const auto& wav = assets_[static_cast<unsigned>(current.sound)];
        const auto count = std::min(std::size_t{64}, wav.sample_count - position);
        for (std::size_t i = 0; i < count; ++i) {
            const auto sample = pcmSample(wav.samples + (position + i) * 2, tuning::AudioVolumeQ15);
            output[i * 2] = sample; output[i * 2 + 1] = sample; // Same mono signal in both I2S slots.
        }
        std::size_t written = 0;
        const auto error = i2s_channel_write(channel_, output.data(), count * 4, &written, 20);
        if (error != ESP_OK || written != count * 4) {
            ready_.store(false); active = false; completed_.store(current.token);
            ESP_LOGW("audio", "Playback failed: %s; continuing silently", esp_err_to_name(error));
            continue;
        }
        position += count;
        if (position == wav.sample_count) {
            active = false; draining = true; drain_start = nowMs();
        }
    }
}
} // namespace portal
