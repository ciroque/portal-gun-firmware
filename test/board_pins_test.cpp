#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "portal/pins.hpp"

#include <array>
#include <algorithm>

TEST_CASE("prop signals are distinct and avoid reserved board functions") {
    using namespace portal::pins;
    auto signals = std::array{Trigger, ColorButton, CurveButton, DirectionButton,
        NeoPixelData, ChargeLed1, ChargeLed2, ChargeLed3, MuzzleReserved,
        I2sBclk, I2sLrclk, I2sData};
    std::sort(signals.begin(), signals.end());
    CHECK(std::adjacent_find(signals.begin(), signals.end()) == signals.end());
    for (int gpio : signals) {
        CHECK_FALSE(gpio == 0);
        CHECK_FALSE(gpio == 3);
        CHECK_FALSE(gpio == 19);
        CHECK_FALSE(gpio == 20);
        CHECK_FALSE((gpio >= 26 && gpio <= 37));
        CHECK_FALSE((gpio >= 39 && gpio <= 46));
        CHECK_FALSE(gpio == 48);
    }
}
