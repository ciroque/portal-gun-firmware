# Architecture decision records

ADRs capture a consequential decision, its reason, and its tradeoffs. Keep each
record short; implementation details belong in code or design documents.

These initial records capture decisions already stated in
[the project handoff](../THE-NEW-HOTNESS.md). They do not represent a completed
architecture proposal or hardware validation.

| ADR | Decision | Status |
| --- | --- | --- |
| [0001](0001-native-esp-idf.md) | Use ESP32-S3 with native ESP-IDF and C++ | Accepted |
| [0002](0002-preserve-existing-behavior.md) | Preserve the sketch's behavior during migration | Accepted |
| [0003](0003-controller-coordinates-subsystems.md) | Coordinate subsystems through a responsive controller | Accepted |
| [0004](0004-persist-settings-in-nvs.md) | Persist validated, versioned settings in NVS | Accepted |
| [0005](0005-removable-battery-and-hardware-power.md) | Use a removable battery and physical power disconnect | Accepted |
| [0006](0006-synchronized-lighting-and-muzzle.md) | Synchronize charge indicators and muzzle with firing phases | Accepted; muzzle portion superseded by 0009 |
| [0007](0007-native-audio-and-standalone-operation.md) | Prefer native audio and keep operation independent of networking | Accepted |
| [0008](0008-gpio-allocation.md) | Allocate GPIOs while preserving programming and debug interfaces | Accepted; board photos checked |
| [0009](0009-defer-muzzle-emitter.md) | Defer muzzle emitter; retain driver selection and procurement history | Accepted |

Use sequential numbers and descriptive filenames. New decisions can start as
**Proposed** and become **Accepted** when agreed. Preserve old records when a
decision changes: mark them **Superseded by ADR-NNNN** and link the replacement.

Still open: audio
amplifier and asset format, charge-stage thresholds, and the detailed component
layout. Record these when decided.
