# ADR-0008: Allocate GPIOs while preserving programming and debug interfaces

- Status: Accepted; physical board markings checked against photos
- Date: 2026-10-25

## Context

The prop needs four inputs and eight outputs, including three I2S signals.
Chip capabilities alone do not establish development-board pin availability.

## Decision

Use this allocation for the documented Lonely Binary ESP32-S3 N16R8 board.
Numbers are GPIO identifiers, not header positions.

| Signal | GPIO | Use |
| --- | --- | --- |
| Trigger | 4 | Active-low input, internal pull-up |
| Portal color button | 5 | Active-low input, internal pull-up |
| Acceleration curve button | 6 | Active-low input, internal pull-up |
| Discharge direction button | 7 | Active-low input, internal pull-up |
| NeoPixel data | 8 | Output through AHCT buffer |
| Charge LED 1 | 9 | Output |
| Charge LED 2 | 10 | Output |
| Charge LED 3 | 11 | Output |
| Muzzle enable | 12 | Reserved; subsystem deferred by ADR-0009 |
| I2S bit clock | 15 | Output |
| I2S word select / LRCLK | 16 | Output |
| I2S audio data | 17 | Output |

Reserve GPIO19/20 for native USB, 43/44 for UART0, and 39–42 for external
JTAG. Avoid strapping GPIO0/3/45/46 and memory GPIO26–37.
Leave GPIO48's onboard RGB LED unused by prop wiring.

## Consequences

Centralize these assignments in firmware. I2S needs no MCLK allocation initially;
revisit if the selected amplifier requires it. Other alternate pin functions do
not reserve these GPIOs when unused.

The supplied top/bottom photos identify a Lonely Binary Gold Edition
`ESP32-S3-DEVKITC-1 V1.6` board with an `ESP32-S3-WROOM-1` module marked
`MCN16R8`. The header labels expose every allocated GPIO (4–12 and 15–17),
and the onboard RGB LED is labeled `RGB@IO48`. These visible markings support
the allocation above; photos do not establish electrical continuity or validate
peripheral operation.

Keep the muzzle disabled during initialization and provide an external pull-down
at its switching-control input so reset does not depend on firmware. Current
limiting, LED resistors, and power validation remain separate decisions.

## References

- [Espressif ESP32-S3 Series Datasheet, v2.2](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf),
  sections 2.3.4–2.3.5 (GPIO restrictions and peripheral assignment), 2.6
  (memory connections), and 3 (boot configuration).
- [Lonely Binary ESP32-S3 board pinout](https://learn.lonelybinary.com/pinouts/esp32-s3),
  for exposed GPIOs, N16R8 PSRAM restrictions, and onboard connections.
- Actual project board: [top photo](../docs/ESP32-S3-top.JPG) and
  [bottom photo](../docs/ESP32-S3-bottom.JPG), inspected 2026-10-25.

Consulted 2026-10-25. The Espressif document describes the chip; the vendor
reference supplies board-specific information.
