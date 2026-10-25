# Firmware playback audio

Regenerate the four active playback WAVs from the untouched `wav/` sources:

```sh
python3 tools/prepare_audio.py
```

Outputs are mono, 16-bit PCM at 44.1 kHz. Stereo recordings are downmixed by
averaging left and right channels without boosting gain. Mono recordings retain
their sample data. Durations are preserved; source metadata is not copied.
The disabled ambient recording is intentionally excluded.

See [ADR-0015](../../adr/0015-audio-format-and-storage.md).
