#pragma once
#include "portal/settings.hpp"
#include "nvs.h"

namespace portal {
/// NVS persistence adapter. Storage errors leave the runtime on safe defaults.
class SettingsStore {
public:
    ~SettingsStore();
    /// Initialize NVS and load validated settings, defaulting when unavailable.
    Settings load();
    /// Persist a validated value; returns false if the write or commit fails.
    bool save(const Settings& settings);
    SettingsStore() = default;
    SettingsStore(const SettingsStore&) = delete;
    SettingsStore& operator=(const SettingsStore&) = delete;
private:
    nvs_handle_t handle_ = 0;
};
} // namespace portal
