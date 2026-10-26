# ADR-0004: Persist validated, versioned settings in NVS

- Status: Accepted
- Date: 2026-10-25

## Context

The sketch stores portal color, acceleration curve, and discharge direction in
AVR EEPROM. These settings must survive power cycles on the ESP32-S3.

## Decision

Use ESP-IDF NVS with an explicit configuration version or namespace strategy.
Validate stored values before use. Missing, invalid, or incompatible data falls
back to safe defaults: blue portal, linear acceleration, and direction `-1`.
Immediate persistence after an accepted settings change is acceptable.
If NVS initialization reports `ESP_ERR_NVS_NO_FREE_PAGES` or
`ESP_ERR_NVS_NEW_VERSION_FOUND`, erase and reinitialize the dedicated NVS
partition before loading defaults. Other NVS errors use volatile defaults and
must not trigger an erase.

## Consequences

EEPROM storage assumptions do not carry into the new implementation. Schema
changes require migration or a safe reset. Configuration problems must not
prevent operation, and validation can be tested independently of NVS. Resetting
the NVS partition on those two initialization errors is safe because this
firmware's partition table reserves NVS exclusively for the prop.
