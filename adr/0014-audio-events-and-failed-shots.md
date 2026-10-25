# ADR-0014: Map sound events and add occasional failed shots

- Status: Accepted
- Date: 2026-10-25

## Context

The owner supplied eleven PCM WAV files in `wav/` and accepted the initial event
mapping below. Filenames and WAV metadata were inspected; the files have not
been auditioned during this review.

## Decision

| Event | Asset | Measured duration | Behavior |
| --- | --- | --- | --- |
| Startup | `portalgun_powerup1.wav` | 1.308 s | Play once at power-on; an accepted trigger press interrupts it |
| Charging / spin-up | None | — | Silent for the initial implementation |
| Blue blast | `portalgun_shoot_blue1.wav` | 0.700 s | Begin at BLAST entry |
| Orange blast | `portalgun_shoot_red1.wav` | 0.822 s | Begin at BLAST entry; use the supplied red-named asset for orange |
| Idle ambience | `portal_ambient_loop1.wav` | 4.666 s | Retained for future use; disabled initially |
| Failed shot | `portal_invalid_surface3.wav` | 0.990 s | Replace a normal firing sequence on a rare accepted trigger press |

Allow firing sounds to finish across discharge/decay instead of truncating them
at the end of the 240 ms blast. Initially use visual settings feedback alone.
Charging is intentionally silent for now: do not start or continue idle ambience
during spin-up, or repurpose the startup sound as a charge effect.

On each accepted idle trigger press, make one random choice with a **5% failed-shot
probability**. A failed shot plays the invalid-surface sound and gives a brief
flicker in the current portal color, then returns to idle. It performs no spin-up
or white blast, changes no settings, and requires trigger release and another
press for another attempt. Do not reroll while held or queue presses received
during an effect. Exact flicker duration/pattern remains to be tuned.

This is a deliberate behavior extension to the reference sketch. It is cosmetic;
there is no surface sensor or actual surface-validity test. The controller owns
the failed-shot outcome and coordinates its lighting and sound.

## Playback priority and volume

Use one playback voice, without mixing or queued sounds. Startup plays once but
does not prevent an accepted trigger press: cancel startup playback immediately
and begin the selected normal or failed-shot outcome. Normal spin-up is silent.

Firing and failed-shot sounds play to completion. Ignore trigger presses during
the active effect. Treat a shot as busy until both its visual sequence and sound
complete, so audio cannot overlap a subsequent shot; normal firing audio is
expected to finish within discharge/decay. A missing asset or playback failure
must not leave the controller waiting indefinitely.

Disable idle ambience initially. Start with a conservative fixed software volume
and tune it on the bench with the actual speaker, assets, and supply. No physical
volume control is required for this implementation.

## Deferred choices

- Charge audio is deferred. No loop extraction, time stretching, or additional
  charge asset is required for the initial implementation. The power-up asset
  remains assigned to startup.
- The owner subsequently requested removal of unused assets. Deleted
  `portal_open1.wav`, `portal_open2.wav`, `portal_open3.wav`, `portal_close1.wav`,
  `portal_close2.wav`, and `portal_fizzle2.wav`; no runtime events were assigned
  to them. The five mapped assets above remain, including optional ambience.
- Future idle ambience enablement, exact volume level, and audio-failure timeout
  details remain implementation decisions. Asset format and storage are specified
  in [ADR-0015](0015-audio-format-and-storage.md).
- The hardware master switch removes power; no normal shutdown sound is assigned.

## Consequences

The initial event set stays small, with an occasional playful failure replacing
a successful shot. Controller tests should inject the random outcome so both
paths, ignored busy-state presses, and release/repress behavior are deterministic.
Verify the actual sounds and loop boundary before final tuning.

The five retained files are 16-bit PCM WAV; the ambient file is mono at 11025 Hz,
the others are 44100 Hz, and the two firing files are stereo. Preserve these
source assets when preparing playback versions.

## References

- [Source assets](../wav/).
- [ADR-0002: behavioral preservation](0002-preserve-existing-behavior.md).
- [ADR-0010: audio hardware](0010-audio-hardware.md).
- [ADR-0013: visual settings feedback](0013-settings-feedback.md).
