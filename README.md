# Portal gun firmware

Migration of the original Nano Every prop sketch to native ESP-IDF/C++ on the
Lonely Binary ESP32-S3 N16R8. Architecture and behavior decisions are in
[adr/](adr/README.md); the original [firmware.ino](firmware.ino) remains the
behavioral reference and is not compiled by ESP-IDF.

The current scaffold boots and logs a message. Ring effects, inputs, persistence,
and audio runtime are not implemented yet. No prop GPIOs are driven by this
scaffold, and the deferred muzzle pin remains reserved.

## Host tests

Requires CMake 3.22+, a C++17 compiler, a build tool, Python 3, and Git. No ESP-IDF,
network download, or attached hardware is needed; doctest is vendored.

```sh
cmake -S test -B build/host
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

The host suite checks exact ADR-0008 assignments, valid GPIO ranges, pin conflicts,
and the SDK-version guard. Temporary Git checkouts exercise clean builds and
rejection of staged/unstaged SDK edits or tag changes after configuration.
Behavioral characterization tests will accompany the effect migration.

[GitHub Actions](.github/workflows/host-tests.yml) runs this host suite on all pull
requests (including stacked PRs) and pushes to `main`, with a manual-run option after
the workflow is merged. The `Host tests (Linux)` job covers the board contract
and SDK-guard fixtures using Ubuntu's host tools, without installing ESP-IDF or
accessing hardware. CI uses CMake's default generator and treats compiler warnings
as errors (the CI-only switch requires CMake 3.24+). New PR runs cancel superseded
runs for that PR; every main/manual run retains its own result. Firmware
cross-compilation and bench checks remain separate.

## Firmware build

Install and activate the pinned SDK as described in
[development setup](docs/development-setup.md), then run from the repository root:

```sh
idf.py -B build/firmware build
```

The default target is `esp32s3`, selected through `sdkconfig.defaults`. Defaults
select 16 MB QIO flash, 80 MHz octal PSRAM,
a 1 kHz FreeRTOS tick, and UART0 logging. Configuration and every subsequent build
reject a different SDK release, prerelease, or modified SDK checkout. Change the pin deliberately when
upgrading and install that SDK's matching tools.

The application and host tests explicitly compile as C++17 without language
extensions. Apply the same standard requirement to portable components as they are
introduced; ESP-IDF's own components retain their SDK-selected language standard.

The custom partition table reserves NVS and a 4 MB factory application. Offsets
are assigned automatically so the table can follow a changed partition-table
offset. No unused PHY-data partition is reserved.
Remaining flash is unallocated; there are no OTA slots or audio filesystem.
Normalized audio will later be linked into the application. This layout fits
the scaffold and provides room for the four selected sounds; future firmware
builds must continue to check application size.

Build success does not validate PSRAM operation, wiring, lighting, sound, or the
provisional power budget. These need the actual board and bench checks.
QIO has not yet been tried on the physical board. If the first bench flash fails
to boot, include flash-mode compatibility in the boot-log investigation before
treating it as an application failure.

See [CONTRIBUTING.md](CONTRIBUTING.md) for branches, commits, and PRs.
