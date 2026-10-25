# ADR-0016: Separate portable effect logic from ESP-IDF hardware adapters

- Status: Accepted architecture baseline; implementation and bench validation pending
- Date: 2026-10-25
- Refines: ADR-0003

## Context

`firmware.ino` combines timing, state transitions, rendering, GPIO, and EEPROM.
The handoff requires an architecture proposal before migration. Preserve its
effect calculations and scheduling while adding the accepted indicator,
feedback, failed-shot, and audio behavior without rebuilding a monolithic sketch.

## Decision

Use a portable C++ core with explicit hardware dependencies, a thin composition
root, and two application execution contexts: controller and audio. Use small
interfaces at hardware boundaries; avoid an event bus, per-component tasks, or
a framework of generic subsystems.

### Component boundaries and sketch migration

| Component | Responsibility | Existing code that moves here |
| --- | --- | --- |
| `controller` | Sole owner of phase changes, accepted settings events, frame counters/deadlines, charge stages, feedback, and shot outcomes | `loop()` dispatch, `fire()`, settings-toggle policy, state/timing portions of `updateIdle/Spinup/Blast/Discharge/Decay()` |
| `inputs` | Four active-low inputs, stable-state debounce, press events, no held-button repeat | `updateButtons()` and its debounce state |
| `lighting` | Portable palettes, frame composition, RGB helpers, and a ring/charge-output adapter | `setPortalPalette()`, rendering portions of the five updates, `calculateSpinDelay()`, `wrapPixel()`, `blendColors()`, `scaleColor()`, `fillRing*()` |
| `config` | Runtime settings model, defaults, validation, versioned NVS load/save adapter | `Config`, `loadConfig()`, `isConfigValid()`, `saveConfig()` |
| `audio` | Embedded WAV lookup/parsing, software volume, cancellable single-voice playback through I2S | New; follows ADR-0014 and ADR-0015 |
| `platform` | Central board GPIOs, monotonic clock and random-source adapters | Pin definitions, `millis()` and random outcome dependencies |
| `main` | Initialize adapters, load settings, wire dependencies, start execution | `setup()` composition responsibilities |

Portable headers and calculations must not require ESP-IDF types or hardware to
compile. Keep runtime state with its owner rather than in shared globals. The
controller owns the current settings; storage persists snapshots. Centralize the
reference tuning values, with palettes/effect math in lighting. Components use
ESP-IDF logging at hardware boundaries, without per-frame normal-level logging.

### API shape

These illustrate responsibilities, not frozen C++ declarations:

```cpp
inputs.poll(now);                    // Debounced edges, in reference button order
controller.onButton(event, now);     // Policy, settings and transitions
controller.tick(now);                // Owns deadlines and effect advancement
lighting.present(frame, chargeStage);// Ring pixels and three indicators
audio.play(sound, playbackId);       // Nonblocking request
audio.stop();                        // Nonblocking cancellation
configStore.save(settings);          // Explicit persistence result
```

Lighting produces a 12-pixel frame from the controller's phase and effect
progress; it never advances a phase independently. The controller uses the same
spin-delay calculation to schedule reference frames and derive the indicator
duration. Feedback is an interruptible idle overlay. Failed shots are an
explicit busy outcome, not a palette change or simulated surface sensor.

### Execution and synchronization

- Run the controller loop in the IDF main task after initialization. Poll inputs,
  process accepted events, advance due effects, and present changed output, then
  yield. Choose the tick configuration and loop cadence to resolve 10 ms spin
  frames and 18 ms effect frames without rounding them to coarse task delays.
- Preserve reference scheduling: advance one due animation frame per update,
  record the current time, and do not add catch-up frames that change effect feel.
  Preserve the debounce comparison and trigger-first input ordering initially.
- The audio task alone owns I2S playback. Transfer play/stop intent through a
  bounded, replaceable command mechanism, never an unbounded playback queue.
  Bound output-write waits so cancellation remains responsive. Only already
  buffered samples may remain briefly audible after cancellation; measure that
  latency on hardware and keep buffers small.
- Tag playback requests and completion/failure notifications so stale startup
  completion cannot finish a later shot. The controller observes audio status
  without sharing unsynchronized playback state. Bound completion waits using
  the asset duration plus an explicit timeout margin; failures release the shot.
- Start firing audio and the white ring from the same BLAST transition. Digital
  commands share a controller timestamp; physical audio/LED latency still requires
  bench verification. No separate task independently times firing effects.

NVS writes remain synchronous initially because accepted settings changes are
infrequent. Perform them outside frame transmission, report failures, and retain
the usable runtime selection. Measure write latency; if it materially delays
trigger handling, move persistence behind a bounded worker rather than blocking
animation or audio tasks indefinitely.

### Repository layout

```text
CMakeLists.txt
sdkconfig.defaults
partitions.csv
main/                    # app_main and dependency composition
components/
  controller/
  inputs/
  lighting/
  config/
  audio/
  platform/
test/                    # Host tests for portable logic
tools/prepare_audio.py   # Existing reproducible conversion
assets/audio/            # Existing four normalized playback WAVs
wav/                     # Original recordings
firmware.ino             # Preserved behavioral reference; not built by IDF
```

Each IDF component has a CMake registration and public headers as needed. Host
tests compile portable sources directly. Do not add a power-management task,
muzzle component, network service, or unused hardware abstractions initially.
Keep GPIO12 reserved as specified by ADR-0009.

## Migration constraints and verification

Replace Arduino GPIO, NeoPixel transmission, timing, EEPROM, and `map()` with
IDF adapters or explicit portable equivalents. Preserve integer interpolation,
brightness scaling, signed pixel wrapping, curve-delay truncation, and frame
endpoint behavior. Use explicit-width arithmetic where AVR and ESP32 types differ;
compare curve outputs to the reference before accepting the port.

GPIO allocation is ADR-0008; audio hardware/format are ADR-0010/0015. Confirm
flash/PSRAM settings for the actual N16R8 module, and size partitions from the
linked image. The selected ring-output implementation must coexist with I2S.

Exact CPU execution and transmission latency cannot be identical across MCUs.
Preserve commanded RGB values, durations, and scheduling semantics; verify the
physical result on the ring. Intentional changes are the nonblocking feedback
and color pulse (ADR-0013), charge stages (ADR-0012), audio, and 5% failed shots
(ADR-0014). Do not silently retune effects to accommodate the port.

Implement and test the portable core first, then IDF composition/adapters and
audio integration. Host checks cover frame values, delay curves, debounce/held
presses, phase boundaries, settings validation, feedback interruption, charge
thresholds, deterministic injected shot outcomes, and audio completion/failure.
IDF builds verify integration; actual I/O, audio quality, synchronization, and
the provisional power budget (ADR-0011) require bench testing.
The test frameworks and TDD workflow are specified in
[ADR-0017](0017-test-framework-and-tdd.md).

## Consequences

Effect logic can be verified before hardware is available, while the controller
remains the synchronization authority. There are only two application tasks and
no component needs to know another component's GPIOs. Hardware driver details,
buffer sizes, exact volume, and failed-shot flicker tuning can be resolved during
implementation without new ADRs unless they change an accepted decision.

## References

- [Project handoff](../THE-NEW-HOTNESS.md), especially sections 4–8 and 25.
- [Behavioral reference](../firmware.ino).
- [ADR index](README.md) for the accepted behavior and hardware decisions.
