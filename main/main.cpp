#include "esp_log.h"
#include "portal/pins.hpp"

extern "C" void app_main() {
    ESP_LOGI("main", "Portal gun scaffold ready; NeoPixel GPIO %d, muzzle GPIO %d reserved",
             portal::pins::NeoPixelData, portal::pins::MuzzleReserved);
    // Peripheral initialization and the controller loop arrive in later changes.
}
