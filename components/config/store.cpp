#include "portal/store.hpp"
#include "esp_log.h"
#include "nvs_flash.h"

namespace portal {
SettingsStore::~SettingsStore() { if (handle_) nvs_close(handle_); }
Settings SettingsStore::load() {
    Settings settings;
    auto error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW("config", "NVS format/capacity incompatible; resetting settings partition");
        error = nvs_flash_erase();
        if (error == ESP_OK) error = nvs_flash_init();
    }
    if (error == ESP_OK) error = nvs_open("portal", NVS_READWRITE, &handle_);
    if (error != ESP_OK) {
        ESP_LOGW("config", "NVS unavailable (%s); using volatile defaults", esp_err_to_name(error));
        return settings;
    }
    ConfigBytes bytes{};
    std::size_t size = bytes.size();
    error = nvs_get_blob(handle_, "settings", bytes.data(), &size);
    if (error != ESP_OK || !decodeSettings(bytes.data(), size, settings)) {
        ESP_LOGW("config", "Missing/invalid settings; loading safe defaults");
        settings = Settings{};
        save(settings);
    }
    return settings;
}
bool SettingsStore::save(const Settings& settings) {
    if (!handle_ || !validSettings(settings)) return false;
    const auto bytes = encodeSettings(settings);
    auto error = nvs_set_blob(handle_, "settings", bytes.data(), bytes.size());
    if (error == ESP_OK) error = nvs_commit(handle_);
    if (error != ESP_OK) ESP_LOGW("config", "Save failed: %s", esp_err_to_name(error));
    return error == ESP_OK;
}
} // namespace portal
