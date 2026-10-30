#pragma once
#include "portal/debounce.hpp"
#include <array>

namespace portal {
/// Configures active-low board buttons and reports debounced press edges.
class GpioInputs {
public:
    /// Configure the GPIO input pins with internal pull-ups.
    bool init();
    /// Return press flags in Trigger, Color, Direction, Curve order.
    std::array<bool, 4> poll(std::uint32_t now);
private:
    std::array<Debouncer, 4> buttons_{};
    bool ready_ = false;
};
} // namespace portal
