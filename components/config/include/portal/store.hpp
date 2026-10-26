#pragma once
#include "portal/settings.hpp"
#include "nvs.h"

namespace portal {
class SettingsStore {
public:
    ~SettingsStore();
    Settings load();
    bool save(const Settings& settings);
    SettingsStore() = default;
    SettingsStore(const SettingsStore&) = delete;
    SettingsStore& operator=(const SettingsStore&) = delete;
private:
    nvs_handle_t handle_ = 0;
};
} // namespace portal
