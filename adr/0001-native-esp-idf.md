# ADR-0001: Use ESP32-S3 with native ESP-IDF and C++

- Status: Accepted
- Date: 2026-10-25

## Context

The Nano Every sketch must grow to support additional lighting and audio while
remaining maintainable. The target board is the Lonely Binary ESP32-S3 N16R8.

## Decision

Use native ESP-IDF, its CMake build system, and C++, entering through `app_main()`.
Arduino will not be the application foundation. Introducing Arduino as an IDF
component requires a separate, explicit decision supported by a useful library.

## Consequences

Arduino GPIO, timing, EEPROM, and NeoPixel integration need platform replacements.
The board's actual pinout and restrictions must be verified before assigning
GPIOs; assignments will be centralized.
