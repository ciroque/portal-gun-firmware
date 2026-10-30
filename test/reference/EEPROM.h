#pragma once
// In-memory EEPROM subset used only to compile the unchanged reference sketch.
#include <array>
#include <cstring>
struct ReferenceEeprom {
    std::array<unsigned char, 64> bytes{};
    template<class T> void get(int offset, T& value) { std::memcpy(&value, bytes.data() + offset, sizeof(T)); }
    template<class T> void put(int offset, const T& value) { std::memcpy(bytes.data() + offset, &value, sizeof(T)); }
};
inline ReferenceEeprom EEPROM;
