#include "doctest.h"
#include "portal/wav.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace {
std::vector<std::uint8_t> readAsset(const char* name) {
    const auto path = std::filesystem::path(__FILE__).parent_path().parent_path() / "assets/audio" / name;
    std::ifstream file(path, std::ios::binary);
    REQUIRE(file.good());
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
}
TEST_CASE("the four embedded playback assets are valid mono PCM WAVs") {
    for (const auto* name : {"portalgun_powerup1.wav", "portalgun_shoot_blue1.wav",
                            "portalgun_shoot_red1.wav", "portal_invalid_surface3.wav"}) {
        const auto bytes = readAsset(name);
        const auto wav = portal::parseWav(bytes.data(), bytes.size());
        CAPTURE(name);
        REQUIRE(wav.has_value());
        CHECK(wav->sample_count > 30000);
        CHECK(wav->durationMs() >= 699);
        CHECK(wav->durationMs() < 1400);
    }
}
TEST_CASE("WAV parsing rejects truncated chunks, wrong formats, and missing data") {
    const auto good = readAsset("portalgun_shoot_blue1.wav");
    for (std::size_t size : {0u, 4u, 12u, 43u}) CHECK_FALSE(portal::parseWav(good.data(), size));
    CHECK_FALSE(portal::parseWav(nullptr, 100));
    auto bad = good;
    bad[22] = 2; // Stereo cannot be interpreted as mono PCM.
    CHECK_FALSE(portal::parseWav(bad.data(), bad.size()));
    bad = good; bad[20] = 3; // IEEE float is not integer PCM.
    CHECK_FALSE(portal::parseWav(bad.data(), bad.size()));
    bad = good; bad[34] = 8;
    CHECK_FALSE(portal::parseWav(bad.data(), bad.size()));
    bad = good; bad[24] = 0;
    CHECK_FALSE(portal::parseWav(bad.data(), bad.size()));
    CHECK_FALSE(portal::parseWav(good.data(), good.size() - 1));
}
TEST_CASE("unknown padded RIFF chunks are skipped instead of assuming a 44-byte header") {
    auto bytes = readAsset("portalgun_shoot_blue1.wav");
    const std::uint8_t chunk[] = {'J','U','N','K',1,0,0,0,0x42,0};
    bytes.insert(bytes.begin() + 12, std::begin(chunk), std::end(chunk));
    const auto riff_size = static_cast<std::uint32_t>(bytes.size() - 8);
    for (unsigned i = 0; i < 4; ++i) bytes[4 + i] = (riff_size >> (8 * i)) & 255;
    REQUIRE(portal::parseWav(bytes.data(), bytes.size()).has_value());
}
TEST_CASE("PCM scaling is signed, bounded, and never boosts above unity") {
    const std::uint8_t negative[] = {0, 128};
    const std::uint8_t positive[] = {255, 127};
    CHECK(portal::pcmSample(negative, 8192) == -8192);
    CHECK(portal::pcmSample(positive, 8192) == 8191);
    CHECK(portal::pcmSample(negative, 32768) == -32768);
    CHECK(portal::pcmSample(positive, 65535) == 32767);
    CHECK(portal::pcmSample(negative, 0) == 0);
}
