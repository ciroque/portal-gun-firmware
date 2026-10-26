# Firmware components

`platform` supplies the portable board pin contract. `controller` owns phase and
feedback timing; `inputs` supplies portable debounce plus GPIO polling; `lighting`
supplies portable palettes/renderers and the RMT/charge-output adapter; `config`
supplies the settings codec and NVS adapter; `audio` supplies the portable WAV
parser and single I2S playback task. `main` composes these explicit dependencies.

Portable sources are compiled directly into host tests without ESP-IDF headers.
The audio adapter implements the controller-owned `AudioPort` interface; the
controller does not depend on I2S or the audio task. See
[ADR-0016](../adr/0016-firmware-component-architecture.md).
