# ADR-0017: Use doctest for host TDD and Unity for target-specific checks

- Status: Accepted
- Date: 2026-10-25

## Context

The migration must preserve tuned behavior while adding controller features.
Most timing, input, rendering, and settings logic can run without an ESP32,
making rapid host tests valuable before hardware integration.

## Decision

Use **doctest** with a standalone host CMake build and CTest for portable C++
tests. Compile the same core sources used by firmware, without requiring IDF,
a connected board, or flashing. Pin the framework dependency to a reviewed
release/commit when implementing the harness; do not track a moving branch.

Use ESP-IDF's bundled **Unity** for targeted checks that genuinely require the
device or IDF adapters. Add these when needed rather than duplicating the host
suite on hardware.

For existing behavior, first write characterization tests with expectations
derived from `firmware.ino`. For new behavior, use the red/green/refactor cycle:
write a failing behavioral test, implement the smallest useful change, then
refactor with tests passing. Do not compute expected results by calling the same
production helper being tested.

Inject time, button readings/events, random outcomes, storage results, and audio
completion/failure at explicit boundaries. Advance simulated time rather than
sleeping or depending on nondeterministic random draws.

## Initial coverage

- Palette values, representative animation frames, index wrapping, integer RGB
  arithmetic, and acceleration-delay values against the reference sketch.
- State progression and timing boundaries, especially the 240 ms blast.
- Debounce, held-trigger behavior, ignored busy presses, and settings validation.
- Adaptive charge thresholds and feedback replacement/trigger interruption.
- Successful and failed shots, playback completion, and failure/timeout recovery.

Test observable behavior and meaningful boundaries. Avoid tests that merely
mirror implementation details, exhaustive mocks of ESP-IDF, or coverage quotas.

## Consequences

Core development gets a fast feedback loop and regression protection without
adding test-framework code to the production firmware. The host suite does not
validate electrical wiring, physical LED/audio synchronization, sound quality,
temperature, or supply capacity; those remain bench checks under ADR-0011/0016.

This records the approach only; the test harness is part of implementation.

## References

- [doctest](https://github.com/doctest/doctest) and its
  [CMake integration](https://github.com/doctest/doctest/blob/master/doc/markdown/build-systems.md).
- [ESP-IDF Unity test example](https://github.com/espressif/esp-idf/blob/master/examples/system/unit_test/README.md).
- [ADR-0016: firmware architecture](0016-firmware-component-architecture.md).
- [Behavioral reference](../firmware.ino).
