# Portal gun firmware

Migration of the original Nano Every prop sketch to native ESP-IDF/C++ on the
Lonely Binary ESP32-S3 N16R8. Architecture and behavior decisions are in
[adr/](adr/README.md); the original [firmware.ino](firmware.ino) remains the
behavioral reference and is not compiled by ESP-IDF.

The current scaffold boots and logs a message. Ring effects, inputs, persistence,
and audio runtime are not implemented yet. No prop GPIOs are driven by this
scaffold, and the deferred muzzle pin remains reserved.

## Host tests

Requires CMake 3.22+, a C++17 compiler, a build tool, and Python 3. No ESP-IDF,
network download, or attached hardware is needed; doctest is vendored.

```sh
cmake -S test -B build/host
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

The host suite currently checks the board pin contract and SDK-version guard.
Behavioral characterization tests will accompany the effect migration.

## Firmware build

Install and activate the pinned SDK as described in
[development setup](docs/development-setup.md), then run from the repository root:

```sh
idf.py -B build/firmware build
```

The default target is `esp32s3`. Defaults select 16 MB flash, 80 MHz octal PSRAM,
a 1 kHz FreeRTOS tick, and UART0 logging. Configuration rejects a different SDK
release, prerelease, or modified SDK checkout. Change the pin deliberately when
upgrading and install that SDK's matching tools.

The custom partition table reserves NVS, PHY data, and a 4 MB factory application.
Remaining flash is unallocated; there are no OTA slots or audio filesystem.
Normalized audio will later be linked into the application. This layout fits
the scaffold and provides room for the four selected sounds; future firmware
builds must continue to check application size.

Build success does not validate PSRAM operation, wiring, lighting, sound, or the
provisional power budget. These need the actual board and bench checks.

See [CONTRIBUTING.md](CONTRIBUTING.md) for branches, commits, and PRs.
