# ADR-0015: Embed normalized PCM WAV playback assets in firmware flash

- Status: Accepted
- Date: 2026-10-25

## Context

The four active sounds are short and fit comfortably in the target's flash.
Source WAVs include mono and stereo recordings. Playback must remain independent
of controller execution without requiring additional storage hardware.

## Decision

Prepare playback copies as **mono, 16-bit PCM WAV at 44.1 kHz**. Preserve the
original files in `wav/`; generate normalized copies separately through a
reproducible asset-preparation step. Downmix stereo without summing channels in
a way that clips, and verify the results before final volume tuning.

Embed these four normalized assets using the ESP-IDF build system:

- `portalgun_powerup1.wav`
- `portalgun_shoot_blue1.wav`
- `portalgun_shoot_red1.wav` (orange firing event)
- `portal_invalid_surface3.wav`

Keep `portal_ambient_loop1.wav` as a source asset but exclude it from the initial
firmware build because ambience is disabled by ADR-0014.

Use a dedicated audio task to feed small PCM chunks to I2S. Parse the WAV data
chunk rather than assuming every file has a fixed 44-byte header. Adapt mono
samples to the configured I2S channel/slot layout for the MAX98357A. Controller
commands initiate or cancel playback according to ADR-0014 without waiting for
audio output to finish synchronously.

## Consequences

No SD card, runtime audio filesystem, or compressed-audio decoder is required.
The four normalized files require approximately **337 KB decimal** including
ordinary WAV headers; exact output size depends on generated metadata. Allocate
firmware partition space using the actual linked image size.

Sound replacement initially requires rebuilding/flashing firmware. Source assets
remain available for later changes. Conversion tooling, chunk/buffer sizes, and
the exact I2S slot configuration are implementation details; no asset conversion
or firmware implementation is performed by this record.

## References

- [Source assets](../wav/).
- [ADR-0010: audio hardware](0010-audio-hardware.md).
- [ADR-0014: audio events and playback rules](0014-audio-events-and-failed-shots.md).
