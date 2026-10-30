#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portal/controller.hpp"
#include "portal/audio.hpp"
#include "portal/gpio_inputs.hpp"
#include "portal/ring.hpp"
#include "portal/store.hpp"

// Firmware composition root: bind hardware adapters, sample inputs, and tick the controller.
namespace {
/// Production source of the five-percent failed-shot event.
class HardwareRandom : public portal::ShotRandom {
    /// Reject the modulo-bias tail before mapping entropy to a percentage.
    bool failedShot() override {
        std::uint32_t value;
        do { value = esp_random(); } while (value >= UINT32_MAX - UINT32_MAX % 100);
        return value % 100 < 5;
    }
};
}

/// ESP-IDF application entry point; owns long-lived adapters for the main loop.
extern "C" void app_main() {
    static portal::SettingsStore store;
    static portal::RingLights lights;
    static portal::GpioInputs inputs;
    static portal::I2sAudio audio;
    static HardwareRandom random;
    const auto settings = store.load();
    lights.init(); inputs.init(); audio.init();
    portal::Controller controller(lights, audio, random, settings);
    auto now = [] { return static_cast<std::uint32_t>(esp_timer_get_time() / 1000); };
    controller.begin(now());
    ESP_LOGI("main", "Controller ready; muzzle GPIO remains reserved");
    auto previous_state = controller.state();
    for (;;) {
        const auto time = now();
        const auto presses = inputs.poll(time);
        for (unsigned i = 0; i < presses.size(); ++i)
            if (presses[i]) controller.press(static_cast<portal::Button>(i), time);
        controller.tick(time);
        if (controller.state() != previous_state) {
            ESP_LOGI("controller", "%s -> %s", portal::stateName(previous_state), portal::stateName(controller.state()));
            previous_state = controller.state();
        }
        if (controller.takeSettingsChanged()) {
            const auto& current = controller.settings();
            ESP_LOGI("config", "color=%u curve=%u direction=%d", static_cast<unsigned>(current.color),
                static_cast<unsigned>(current.curve), current.direction);
            if (!store.save(current)) ESP_LOGW("config", "Selection remains active but could not be persisted");
        }
        vTaskDelay(1);
    }
}
