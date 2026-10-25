# ADR-0013: Provide nonblocking curve and color feedback

- Status: Accepted
- Date: 2026-10-25
- Refines: ADR-0002 and ADR-0003

## Context

The sketch identifies the selected acceleration curve with green blinks on
pixel 0, but blocks input during feedback. Color selection changes the idle
palette without a separate acknowledgement.

## Decision

Accept settings changes only while idle, including while settings feedback is
active. Persist accepted changes as before. Provide these ring indications:

| Setting | Feedback |
| --- | --- |
| Acceleration curve | Pixel 0 green, other pixels off; 1–5 blinks for linear, quadratic, cubic, exponential, smoothstep respectively; retain 120 ms on / 120 ms off |
| Portal color | One 200 ms whole-ring pulse using the newly selected palette's `hotColor`, then restart its idle animation |

Implement both indications without blocking waits. A debounced trigger press
cancels feedback and immediately starts the normal firing sequence using the
current settings. Restart idle from its first palette endpoint when feedback
finishes normally.

Use one feedback sequence at a time. A newly accepted curve or color change
replaces the active sequence with feedback for the latest selection; do not
queue indications. Direction changes retain their existing behavior without a
new indication.

## Consequences

The green curve pattern stays familiar while trigger handling becomes responsive.
Color changes gain explicit acknowledgement without additional hardware.
Trigger interruption and the color pulse are deliberate changes from the sketch.
The controller owns feedback scheduling and cancellation; verify completion,
replacement, and trigger interruption in controller logic.

## References

- [Behavioral reference](../firmware.ino): `toggleAccelerationCurve()`,
  `blinkCurveIndicator()`, and `togglePortalColor()`.
- [ADR-0002](0002-preserve-existing-behavior.md).
- [ADR-0003](0003-controller-coordinates-subsystems.md).
