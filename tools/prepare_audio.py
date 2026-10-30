#!/usr/bin/env python3
"""Create playback WAVs without changing the source recordings."""

from pathlib import Path
import struct
import wave


ROOT = Path(__file__).resolve().parents[1]
ACTIVE_SOUNDS = (
    "portalgun_powerup1.wav",
    "portalgun_shoot_blue1.wav",
    "portalgun_shoot_red1.wav",
    "portal_invalid_surface3.wav",
)


def main():
    """Convert the selected original recordings to embedded mono PCM assets."""
    destination = ROOT / "assets" / "audio"
    destination.mkdir(parents=True, exist_ok=True)
    for name in ACTIVE_SOUNDS:
        with wave.open(str(ROOT / "wav" / name), "rb") as source:
            channels = source.getnchannels()
            if (source.getsampwidth() != 2 or source.getframerate() != 44100
                    or channels not in (1, 2) or source.getcomptype() != "NONE"):
                raise ValueError(f"{name}: expected mono/stereo 44.1 kHz 16-bit PCM")
            frames = source.readframes(source.getnframes())

        if channels == 2:
            # Average in integer arithmetic to avoid overflow or gain increase.
            # Round halfway values away from zero, identically on every host.
            samples = []
            for left, right in struct.iter_unpack("<hh", frames):
                total = left + right
                samples.append((total + 1) // 2 if total >= 0
                               else -((-total + 1) // 2))
            frames = struct.pack(f"<{len(samples)}h", *samples)

        with wave.open(str(destination / name), "wb") as output:
            output.setnchannels(1)
            output.setsampwidth(2)
            output.setframerate(44100)
            output.writeframes(frames)
        print(f"{name}: {channels} -> 1 channel, {len(frames) // 2} frames")


if __name__ == "__main__":
    main()
