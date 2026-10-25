# ADR-0006: Synchronize charge indicators and muzzle with firing phases

- Status: Accepted
- Date: 2026-10-25

## Context

The existing 12-pixel ring gains three white charge indicators and a 3 W
warm-white muzzle emitter. Their timing must reinforce the original firing effect.

## Decision

Retain the ring's behavior. Accumulate charge LEDs during spin-up, hold all three
on during blast, and turn them off when discharge begins. Enable the muzzle
emitter with the full-white ring blast, currently 240 ms, and disable it when
discharge begins. The controller coordinates these outputs.

Use an AHCT-family level buffer for the 5 V ring's data input. Drive the muzzle
through an appropriate switching/current-control path; never directly from GPIO.
The current preferred switch is Adafruit #5648. Provide a heat spreader.

## Consequences

Charge-stage thresholds remain to be selected. Muzzle current limiting remains
open; selecting a switch does not resolve it. Final GPIOs, indicator resistors,
and electrical performance require hardware verification.
