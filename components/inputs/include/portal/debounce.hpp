#pragma once
#include <cstdint>

namespace portal {
// Order deliberately matches the sketch: a simultaneous trigger wins over settings.
enum class Button { Trigger, Color, Direction, Curve };
class Debouncer {
public:
    bool poll(bool released, std::uint32_t now) {
        if (released != last_reading_) changed_at_ = now;
        bool pressed = false;
        if (now - changed_at_ > 30 && released != stable_) {
            stable_ = released;
            pressed = !stable_;
        }
        last_reading_ = released;
        return pressed;
    }
private:
    bool last_reading_ = true;
    bool stable_ = true;
    std::uint32_t changed_at_ = 0;
};
} // namespace portal
