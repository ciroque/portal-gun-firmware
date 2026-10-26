#include "portal/gpio_inputs.hpp"
#include "portal/pins.hpp"
#include "driver/gpio.h"
#include "esp_log.h"

namespace portal {
namespace {
constexpr std::array<int, 4> pins_in_order{pins::Trigger, pins::ColorButton, pins::DirectionButton, pins::CurveButton};
}
bool GpioInputs::init() {
    gpio_config_t config{};
    for (auto pin : pins_in_order) config.pin_bit_mask |= std::uint64_t{1} << pin;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    const auto error = gpio_config(&config);
    ready_ = error == ESP_OK;
    if (!ready_) ESP_LOGE("inputs", "GPIO setup failed: %s", esp_err_to_name(error));
    return ready_;
}
std::array<bool, 4> GpioInputs::poll(std::uint32_t now) {
    std::array<bool, 4> presses{};
    if (ready_) for (unsigned i = 0; i < presses.size(); ++i)
        presses[i] = buttons_[i].poll(gpio_get_level(static_cast<gpio_num_t>(pins_in_order[i])) != 0, now);
    return presses;
}
} // namespace portal
