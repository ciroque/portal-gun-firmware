# ADR-0003: Coordinate subsystems through a responsive controller

- Status: Accepted
- Date: 2026-10-25

## Context

Ring effects, charge LEDs, muzzle output, and audio must agree on the current
firing phase without blocking input processing.

## Decision

The controller owns transitions through idle, spin-up, blast, discharge, and
decay. Subsystems expose intent-oriented commands and respond to controller
state. Keep pure calculations separate from hardware access where practical.

Start with a simple controller execution loop/task. Add a separate audio task
only if useful; do not create a task for each component. Avoid long blocking
waits and introduce queues or timers only where they improve correctness or clarity.

## Consequences

Timing and synchronization have a common owner, and pure logic can be tested on
the host. Component APIs and repository layout still need a proposal. Replacing
the sketch's blocking curve-feedback blink must explicitly address input behavior
while retaining its visible feedback.
