# Portal gun firmware

Migration of the original Nano Every prop sketch to native ESP-IDF/C++ on the
Lonely Binary ESP32-S3 N16R8. Architecture and behavior decisions are in
[adr/](adr/README.md); the original [firmware.ino](firmware.ino) remains the
behavioral reference and is not compiled by ESP-IDF.

The application implements ring effects, debounced controls, persistent settings,
three charge LEDs, settings feedback, and synchronized native I2S audio. The
deferred muzzle GPIO12 remains reserved and is never configured by the application.
This is ready for its first hardware validation, not yet a bench-verified build.

## Host tests

Requires CMake 3.22+, a C++17 compiler, a build tool, Python 3, and Git. No ESP-IDF,
network download, or attached hardware is needed; doctest is vendored.

```sh
cmake -S test -B build/host
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

The host suite checks exact ADR-0008 assignments, valid GPIO ranges, pin conflicts,
the controller, debounce, configuration validation, PCM/WAV handling, and the
SDK-version guard. Temporary Git checkouts exercise clean builds and
rejection of staged/unstaged SDK edits or tag changes after configuration.
The unmodified sketch is compiled with host-only I/O stubs as a behavioral oracle.
Tests compare every palette frame and curve delay, plus complete firing timelines
for both colors, five curves, and both directions, including late updates.

[GitHub Actions](.github/workflows/host-tests.yml) runs this host suite on all pull
requests (including stacked PRs) and pushes to `main`, with a manual-run option after
the workflow is merged. The `Host tests (Linux)` job runs all portable logic and
SDK-guard fixtures using Ubuntu's host tools, without installing ESP-IDF or
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
The four normalized playback WAVs are linked into the application. This layout
provides room for the runtime and its sounds; future firmware
builds must continue to check application size.

Build success does not validate PSRAM operation, wiring, lighting, sound, or the
provisional power budget. These need the actual board and bench checks.
QIO has not yet been tried on the physical board. If the first bench flash fails
to boot, include flash-mode compatibility in the boot-log investigation before
treating it as an application failure.

## Runtime and first bench validation

The controller runs in the IDF main task at a 1 ms polling cadence. Ring output
uses hardware RMT at 800 kHz with GRB ordering and a 300 µs latch; a frame waits at most
5 ms for transmission. Only changed ring frames are transmitted. Audio uses a
single worker and I2S DMA, with no mixing, idle hum, or charge sound.

- Trigger: starts spin-up, or a 5% cosmetic failed shot. Holding does not repeat.
- Color: toggles the palette, persists it, and pulses the new hot color for 200 ms.
- Curve: cycles five curves, persists it, and blinks pixel 0 green 1–5 times.
- Direction: reverses discharge and persists it, without extra feedback.

Settings are accepted only while idle. Trigger interrupts settings/startup feedback.
Charge LEDs advance at one-third and two-thirds of the selected spin-up duration;
all three light at the actual 240 ms blast and turn off when discharge starts.
Failed shots use two 50 ms portal-colored pulses separated by 50 ms off periods,
with no charge/blast. That flicker pattern is an initial bench-tuning value.

Settings use an explicit versioned 8-byte NVS record in namespace `portal`, key
`settings`. Invalid data loads defaults; storage failure leaves settings usable in
RAM. NVS is not automatically erased. Immediate commits can introduce latency:
check trigger response and concurrent startup audio when changing settings.

Audio starts at 25% digital amplitude with the amplifier's default hardware gain.
Reference timing and initial bench defaults are centralized in
`components/platform/include/portal/tuning.hpp`; palettes remain in lighting.
The worker uses a one-command mailbox, tagged completions, small DMA buffers,
and bounded writes. Cancellation has a short buffered tail; measure it on the
speaker. A sound's measured duration plus a 250 ms margin bounds controller waits
(with a 10 s duration cap). Invalid assets or audio failure leave lighting usable.
Original WAVs are unchanged; regenerate playback copies with
`python3 tools/prepare_audio.py` when needed.

Before declaring hardware operation verified, identify the correct connected
board with the owner, check the level buffer/LED resistors and common grounds,
then validate boot/PSRAM, both palettes, inputs/debounce, power-cycle persistence,
charge timing, sound/interruptions, and repeated effects at low battery. Confirm
ring timing, audio gain/latency, and the ADR-0011 power budget on the bench.
No device access or flashing was performed during implementation.

See [CONTRIBUTING.md](CONTRIBUTING.md) for branches, commits, and PRs.
