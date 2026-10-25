# ADR-0005: Use a removable battery and physical power disconnect

- Status: Accepted
- Date: 2026-10-25

## Context

The prop needs standalone power and an unambiguous off state. In-prop charging
is not required.

## Decision

Use the planned removable, protected single-cell Adafruit #1781 battery with
external charging through the planned #4410 charger. Place the latching Nilight
90252B-A switch before the planned Pololu #4085 S13V20F5 regulator.

The regulated 5 V bus supplies the board through its appropriate input and the
5 V peripherals, with common grounds. The master switch is a hardware disconnect,
not an MCU input.

## Consequences

[ADR-0011](0011-power-budget.md) records the initial load budget and outstanding
capacity checks; the planned parts are not yet validated for the combined load.

Firmware does not implement charging or logical master-power handling. Final
wiring, regulator capacity under combined loads, and component availability still
require confirmation; this decision is not an electrical validation.
