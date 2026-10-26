#pragma once
#include "portal/debounce.hpp"
#include <array>

namespace portal {
class GpioInputs {
public:
    bool init();
    std::array<bool, 4> poll(std::uint32_t now);
private:
    std::array<Debouncer, 4> buttons_{};
    bool ready_ = false;
};
} // namespace portal
