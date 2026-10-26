#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "portal/pins.hpp"

#include <array>
#include <algorithm>

// Expected values are the reviewed physical wiring contract, not derived from
// the production constants: a unique-but-wrong assignment must fail too.
TEST_CASE("board allocation matches ADR-0008") {
    using namespace portal::pins;
    CHECK(Trigger == 4);
    CHECK(ColorButton == 5);
    CHECK(CurveButton == 6);
    CHECK(DirectionButton == 7);
    CHECK(NeoPixelData == 8);
    CHECK(ChargeLed1 == 9);
    CHECK(ChargeLed2 == 10);
    CHECK(ChargeLed3 == 11);
    CHECK(MuzzleReserved == 12);
    CHECK(I2sBclk == 15);
    CHECK(I2sLrclk == 16);
    CHECK(I2sData == 17);
}

TEST_CASE("prop signals are distinct and avoid reserved board functions") {
    using namespace portal::pins;
    auto signals = std::array{Trigger, ColorButton, CurveButton, DirectionButton,
        NeoPixelData, ChargeLed1, ChargeLed2, ChargeLed3, MuzzleReserved,
        I2sBclk, I2sLrclk, I2sData};
    std::sort(signals.begin(), signals.end());
    CHECK(std::adjacent_find(signals.begin(), signals.end()) == signals.end());
    for (int gpio : signals) {
        CHECK(((gpio >= 0 && gpio <= 21) || (gpio >= 26 && gpio <= 48)));
        CHECK_FALSE(gpio == 0);
        CHECK_FALSE(gpio == 3);
        CHECK_FALSE(gpio == 19);
        CHECK_FALSE(gpio == 20);
        CHECK_FALSE((gpio >= 26 && gpio <= 37));
        CHECK_FALSE((gpio >= 39 && gpio <= 46));
        CHECK_FALSE(gpio == 48);
    }
}
