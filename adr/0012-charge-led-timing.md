# ADR-0012: Space charge LED transitions across the spin-up duration

- Status: Accepted
- Date: 2026-10-25

## Context

Spin-up duration varies with the selected acceleration curve and timing settings.
Equal frame-count intervals are not equal time intervals because frame delays
decrease throughout spin-up.

## Decision

Space charge-stage transitions evenly in time across the selected spin-up
duration. Derive that duration from the actual frame-delay schedule, including
its integer rounding, rather than hardcoding seconds or frame thresholds.

| Spin-up progress in time | Indicators |
| --- | --- |
| Start | All off |
| One third | First on |
| Two thirds | First and second on |
| Complete / blast begins | All three on |
| Discharge begins | All off |

The controller supplies the shared timing and owns phase transitions. Clamp
spin-up indication to two LEDs until the actual BLAST transition, so scheduling
delays cannot indicate completion before the ring fires. Preserve the ring's
reference frame scheduling rather than changing it to fit the indicators.

## Consequences

Changes to the curve, frame count, or delay settings automatically adjust the
indicator timing. No separate indicator task or independent firing timer is
needed. Verify thresholds and phase overrides in pure controller logic.

Refines the charge-indicator timing left open in
[ADR-0006](0006-synchronized-lighting-and-muzzle.md).
