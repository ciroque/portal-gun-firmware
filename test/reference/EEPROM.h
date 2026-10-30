#pragma once
// In-memory EEPROM subset used only to compile the unchanged reference sketch.
#include <array>
#include <cstring>
/// Fixed-size EEPROM stand-in used by the reference sketch's settings routines.
struct ReferenceEeprom {
    std::array<unsigned char, 64> bytes{};
    /// Copy a sketch-compatible value from the in-memory EEPROM image.
    template<class T> void get(int offset, T& value) { std::memcpy(&value, bytes.data() + offset, sizeof(T)); }
    /// Copy a sketch-compatible value into the in-memory EEPROM image.
    template<class T> void put(int offset, const T& value) { std::memcpy(bytes.data() + offset, &value, sizeof(T)); }
};
/// Global compatibility object expected by firmware.ino.
inline ReferenceEeprom EEPROM;
