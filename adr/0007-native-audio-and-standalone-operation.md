# ADR-0007: Prefer native audio and keep operation independent of networking

- Status: Accepted
- Date: 2026-10-25

## Context

Audio should accompany firing effects without destabilizing the controller.
ESP32 connectivity also offers future configuration and service capabilities.

## Decision

Prefer native ESP32 I2S audio through an amplifier and speaker. Keep the available
DFPlayer-style module as a fallback. Audio playback must not block the controller;
playback events follow controller state. Prefer flash storage for assets where
practical.

Normal operation requires no network. Defer browser configuration, asset upload,
OTA, and other service features. Audio initialization failures or missing assets
must allow trigger and lighting operation to continue.

## Consequences

Amplifier and speaker selection are recorded in [ADR-0010](0010-audio-hardware.md).
Asset format, storage layout, and playback implementation remain open.
Networking adds no initial
runtime dependency or service subsystem.
