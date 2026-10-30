#pragma once
#include "portal/settings.hpp"
#include "nvs.h"

namespace portal {
/// NVS persistence adapter. Storage errors leave the runtime on safe defaults.
class SettingsStore {
public:
    /// Close the NVS handle, if load() acquired one.
    ~SettingsStore();
    /// Initialize NVS and load validated settings, defaulting when unavailable.
    Settings load();
    /// Persist a validated value; returns false if the write or commit fails.
    bool save(const Settings& settings);
    /// Construct a store without opening NVS until load() is called.
    SettingsStore() = default;
    /// NVS handles are owned by one store instance.
    SettingsStore(const SettingsStore&) = delete;
    /// NVS handles are owned by one store instance.
    SettingsStore& operator=(const SettingsStore&) = delete;
private:
    nvs_handle_t handle_ = 0;
};
} // namespace portal
