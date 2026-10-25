# ADR-0002: Preserve the sketch's behavior during migration

- Status: Accepted
- Date: 2026-10-25

## Context

The existing prop already has tuned effects and input semantics. An architectural
refactor could unintentionally change its feel.

## Decision

Treat `firmware.ino` as the behavioral reference. Preserve its palettes, frame
calculations, timing, five acceleration curves, firing phases, debounce, and
settings semantics. New charge LEDs, muzzle output, and audio extend that behavior.

Document deliberate deviations and their reason: hardware differences,
electrical constraints, architectural improvements, or new requirements.
Produce the architecture proposal before implementing the migration.

## Consequences

Extract and verify behavior from the source rather than recreating it from prose.
Preserve idle-only settings changes and single-press firing without queued shots
or automatic repeat. The blocking curve indicator needs an explicit treatment
under the controller responsiveness requirement in ADR-0003. That treatment and
the new color acknowledgement are specified in [ADR-0013](0013-settings-feedback.md).
