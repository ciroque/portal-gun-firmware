# Portal Gun Firmware — Coding Assistant Handoff

## 1. Project Objective

Migrate an existing Portal-inspired prop gun firmware project from an **Arduino Nano Every sketch** to a structured **ESP32-S3 / ESP-IDF / CMake / C++** project.

The existing Arduino sketch is available to you.

**Treat the original sketch as the behavioral source of truth.**

Do not re-invent or approximate existing effects from this document. Inspect the sketch and preserve its current runtime behavior unless a change is explicitly justified by:

- ESP32-S3 hardware differences
- architectural improvements
- electrical constraints
- a deliberate new requirement

This document defines the **target architecture, hardware direction, subsystem boundaries, and migration constraints**.

---

# 2. Target Platform

Target MCU:

**Lonely Binary ESP32-S3 N16R8**

Expected characteristics:

- ESP32-S3
- 16 MB flash
- 8 MB PSRAM
- 3.3 V GPIO
- I2S
- Wi-Fi/Bluetooth available
- native USB

The exact board pinout must be inspected before assigning GPIOs.

Do not invent pin assignments until the board's usable GPIOs and restrictions have been confirmed.

---

# 3. Software Platform

Use:

- **ESP-IDF**
- native ESP-IDF CMake build system
- C++
- FreeRTOS only where useful

Do not build this as a conventional Arduino `setup()` / `loop()` application.

Entry point should be based on:

```cpp
extern "C" void app_main();
```

Arduino should not be the architectural foundation.

If a specific Arduino library later provides substantial value, Arduino may be introduced as an ESP-IDF component, but that should be an explicit decision.

---

# 4. Migration Philosophy

This is primarily a **migration and architectural refactor**, not a behavioral rewrite.

The original sketch already contains working implementations for:

- idle animation
- portal colors
- spin-up
- acceleration curves
- white blast
- discharge/unwind
- decay
- trigger behavior
- settings buttons
- debounce behavior
- persistent configuration

Preserve those semantics initially.

Where the existing sketch contains tuning constants, palettes, durations, or effect details, prefer reading them directly from the sketch rather than copying stale values from this document.

The migration should make the code easier to maintain and extend without changing how the prop currently feels.

---

# 5. Major Subsystems

Organize the project around these logical subsystems:

1. Power
2. Controller / state machine
3. Inputs
4. Lighting
5. Muzzle emitter
6. Audio
7. Persistent configuration
8. Optional future service/configuration interface

The controller should coordinate the subsystems.

Subsystem APIs should be intent-oriented.

Prefer:

```cpp
lighting.startSpinup();
lighting.setChargeStage(2);
audio.playCharge();
muzzle.fire();
```

over direct GPIO manipulation from controller code.

---

# 6. Suggested Repository Structure

A reasonable starting layout:

```text
portal-gun/
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
│
├── main/
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── components/
│   ├── controller/
│   ├── inputs/
│   ├── lighting/
│   ├── audio/
│   ├── muzzle/
│   ├── config/
│   └── platform/
│
├── assets/
│   └── audio/
│
└── test/
```

This is a starting point, not a mandate.

If a simpler structure is preferable initially, explain why.

Do not create unnecessary layers merely for architectural purity.

---

# 7. Controller / State Machine

The existing sketch contains the current state model and behavior.

Preserve it conceptually.

Current phases are approximately:

```text
IDLE
  ↓
SPIN-UP
  ↓
BLAST
  ↓
DISCHARGE
  ↓
DECAY
  ↓
IDLE
```

The controller should own state transitions.

Other subsystems should respond to state changes or commands rather than independently inventing behavior.

The controller should remain responsive at all times.

Avoid long blocking waits.

---

# 8. Concurrency

Do not create one FreeRTOS task per component.

Start simple.

A likely initial architecture may use:

- controller task
- audio task if audio playback benefits from separation

Other functionality can remain event-driven or called from the controller.

Use:

- queues
- event groups
- notifications
- timers

only where they improve clarity or correctness.

Avoid unnecessary shared mutable global state.

---

# 9. User Inputs

Physical controls:

## Master power

Existing:

**Nilight 90252B-A**

This is a latching hardware power switch.

It is not a logical MCU input.

The switch's integrated LED is not required.

---

## Trigger

Planned:

- momentary
- SPST-NO
- approximately 16 mm
- physically larger than settings buttons
- non-illuminated preferred

---

## Settings buttons

Three momentary SPST-NO buttons:

- portal color
- acceleration curve
- discharge/unwind direction

Expected electrical topology:

```text
GPIO
 |
switch
 |
GND
```

Use internal pull-ups.

Therefore:

```text
released = HIGH
pressed  = LOW
```

Preserve the existing sketch's debounce and button semantics unless there is a reason to improve them.

Setting changes should normally be accepted while idle and ignored during firing effects unless the existing sketch specifies otherwise.

---

# 10. Persistent Configuration

Persist at least:

- portal color
- acceleration curve
- unwind/discharge direction

Possible future persisted values:

- volume
- brightness
- selected effect profile

Use ESP-IDF **NVS** rather than AVR EEPROM assumptions.

Use a versioned configuration structure or namespace strategy so invalid or old configuration can be detected.

On invalid configuration:

- load safe defaults
- continue operating

Configuration writes are infrequent enough that immediate persistence after a settings change is acceptable.

---

# 11. Power Architecture

Planned battery:

**Adafruit #1781**
- single-cell Li-ion
- 3.7 V nominal
- 2200 mAh
- removable
- JST-PH
- protected pack

Charging is external.

Planned charger:

**Adafruit #4410 USB-C Micro-Lipo charger**

There is no requirement for in-prop charging.

---

## Power path

```text
Li-ion battery
     ↓
Nilight 90252B-A
     ↓
Pololu S13V20F5
     ↓
regulated 5 V bus
```

Selected regulator:

**Pololu #4085 S13V20F5**

The regulated 5 V rail will power:

- ESP32-S3 board through its appropriate 5 V input
- NeoPixel ring
- audio amplifier
- muzzle subsystem
- other 5 V peripherals if required

All grounds are common.

The physical power switch should be before the regulator so OFF is a real battery disconnect.

---

# 12. Main Lighting Subsystem

Existing hardware:

**Adafruit #1643**
12-pixel NeoPixel ring

The original sketch contains the current:

- palettes
- idle pulse
- spin-up behavior
- white blast
- discharge effect
- decay behavior
- acceleration timing

Preserve these initially.

---

## ESP32 level shifting

The ESP32-S3 uses 3.3 V logic while the NeoPixel ring is powered from 5 V.

Use a proper logic-level buffer.

Preferred class of part:

```text
74AHCT1G125
```

or equivalent AHCT-family buffer.

Expected signal path:

```text
ESP32 GPIO
    ↓
AHCT buffer
    ↓
330–470 Ω series resistor
    ↓
NeoPixel DIN
```

Also place a bulk capacitor near the ring:

```text
~1000 µF
≥10 V
```

across 5 V and GND.

Do not rely on 3.3 V directly driving a 5 V NeoPixel input just because it may work in practice.

---

# 13. Charge Indicator LEDs

Add three ordinary white LEDs between the main body and muzzle.

They should visually communicate energy moving toward the muzzle.

Desired behavior:

```text
Stage 0:  ○ ○ ○

Stage 1:  ● ○ ○

Stage 2:  ● ● ○

Stage 3:  ● ● ●

Blast:    ● ● ● + muzzle emitter
```

The LEDs accumulate rather than chase.

They should turn off when discharge begins.

These were not part of the original sketch and are a new feature.

Use separate GPIOs unless there is a compelling reason to multiplex.

Because ESP32 GPIO is 3.3 V, calculate appropriate resistors for the actual white LED forward voltage.

Target roughly:

```text
5–10 mA per LED
```

unless bench testing suggests otherwise.

---

# 14. Muzzle Emitter

Existing hardware:

**CHANZON 3 W Warm White emitter**

Approximate electrical specification:

```text
3000–3500 K
3.0–3.4 V forward voltage
600–700 mA
3 W class
```

This is the primary muzzle flash.

It must not be driven directly by an MCU GPIO.

---

## Switching

Current preferred switching approach:

**Adafruit #5648 MOSFET Driver**

The previously considered discrete **2N7000 is no longer needed** if the #5648 is used.

Expected architecture:

```text
ESP32 GPIO
     ↓
MOSFET driver
     ↓
muzzle LED current path
```

---

## Current control

The final current-limiting method should be reviewed carefully.

A power resistor may be acceptable because the emitter operates only in a short controlled pulse, but a constant-current solution should be considered if it is practical and compact.

Do not treat the 3 W emitter like a normal indicator LED.

Current target is approximately:

```text
600 mA
```

The existing blast duration is approximately a quarter of a second; read the exact value from the source sketch.

---

## Thermal mounting

Even with low duty cycle:

- mount the emitter to aluminum or another heat spreader
- do not mount the emitter thermally against PLA/PETG alone

---

# 15. Audio Subsystem

A DFPlayer-style module is physically available and can be used as a fallback.

However, with the ESP32-S3, the preferred architecture is native audio:

```text
ESP32-S3
   ↓ I2S
audio amplifier
   ↓
speaker
```

Target speaker:

```text
4 Ω
approximately 3 W
approximately 40–50 mm
```

Prefer a small printed speaker enclosure or chamber rather than leaving the speaker acoustically open inside the prop.

---

## Audio assets

Likely sound events include:

- power-on
- charge/spin-up
- blue portal firing
- orange portal firing
- color-change feedback
- curve-change feedback
- direction-change feedback
- optional idle hum
- optional error feedback

The actual source audio assets will be provided separately.

If practical, store audio assets in ESP32 flash.

Do not make audio playback block controller operation.

---

# 16. Synchronization

The important requirement is synchronization across subsystems.

For example:

```text
trigger press
   ↓
controller enters SPIN-UP
   ├─ ring animation accelerates
   ├─ charge LEDs advance
   └─ charge sound begins

spin-up completes
   ↓
controller enters BLAST
   ├─ NeoPixel ring → full white
   ├─ charge LEDs → all on
   ├─ muzzle emitter → on
   └─ firing sound

blast completes
   ↓
controller enters DISCHARGE
   ├─ muzzle off
   ├─ charge LEDs off
   └─ ring performs configured unwind
```

Lighting and audio should be driven by common controller state rather than loosely timed independent effects.

---

# 17. Optional Future Service Mode

The ESP32-S3 has Wi-Fi, but Wi-Fi must not be required for normal operation.

Potential future service features:

- browser configuration
- volume adjustment
- brightness adjustment
- animation timing
- sound replacement/upload
- hardware test page
- OTA firmware updates
- diagnostics

Treat this as a future subsystem.

Do not allow it to complicate initial core functionality.

---

# 18. Logging

Use ESP-IDF logging.

Prefer:

```cpp
ESP_LOGI(...)
ESP_LOGW(...)
ESP_LOGE(...)
```

Each component should have a useful log tag.

Log important events such as:

```text
controller: IDLE -> SPINUP
controller: SPINUP -> BLAST
audio: play fire_blue
muzzle: enabled
config: acceleration curve changed
```

Do not emit per-frame logs at normal log levels.

---

# 19. Error Handling

Failure of an optional subsystem should not render the prop unusable.

Examples:

- invalid NVS data → use defaults
- missing audio asset → continue without that sound
- audio initialization failure → lighting and trigger should still work
- configuration schema mismatch → migrate or reset safely

Fail gracefully where practical.

---

# 20. Hardware Abstraction

Do not scatter GPIO numbers throughout the code.

Define them centrally.

For example:

```cpp
namespace pins {
    constexpr gpio_num_t Trigger = ...;
    constexpr gpio_num_t ColorButton = ...;
    constexpr gpio_num_t CurveButton = ...;
    constexpr gpio_num_t DirectionButton = ...;

    constexpr gpio_num_t ChargeLed1 = ...;
    constexpr gpio_num_t ChargeLed2 = ...;
    constexpr gpio_num_t ChargeLed3 = ...;

    constexpr gpio_num_t NeoPixelData = ...;
    constexpr gpio_num_t MuzzleEnable = ...;

    constexpr gpio_num_t I2sBclk = ...;
    constexpr gpio_num_t I2sLrclk = ...;
    constexpr gpio_num_t I2sData = ...;
}
```

Final pins are not yet assigned.

Inspect the target board first.

---

# 21. Testing

Separate pure logic from hardware-facing code wherever practical.

Good candidates for host/unit tests:

- state transitions
- acceleration curve calculations
- charge-stage calculation
- pixel index wrapping
- configuration validation
- configuration migration
- timing calculations
- state/action mapping

Hardware-specific code should remain relatively thin.

---

# 22. Coding Style

Use modern C++ where it improves clarity.

Prefer:

- `enum class`
- `constexpr`
- clear ownership
- RAII where applicable
- small cohesive classes
- explicit dependencies
- centralized hardware configuration
- centralized tuning parameters

Avoid:

- giant classes
- uncontrolled global state
- blocking delays
- magic numbers
- excessive template machinery
- abstraction for abstraction's sake

Comments should explain:

- timing assumptions
- electrical constraints
- state-machine decisions
- non-obvious hardware behavior

Do not add comments that merely paraphrase obvious code.

---

# 23. Existing Hardware

Known available hardware includes:

- Lonely Binary ESP32-S3 N16R8
- Arduino Nano Every from the original implementation
- Adafruit #1643 12-pixel NeoPixel ring
- Nilight 90252B-A latching power switches
- CHANZON 3 W warm-white emitters
- DFPlayer-style HW-247A audio module if needed
- various existing electronics and connectors

Additional hardware is being selected separately.

Do not assume final procurement status without checking.

---

# 24. Important Decisions Already Made

Treat these as current project decisions:

- migrate away from Nano Every
- use ESP32-S3
- use native ESP-IDF
- use CMake
- use C++
- original sketch remains behavioral reference
- retain physical master power switch
- use removable externally charged Li-ion battery
- retain three settings buttons
- retain separate trigger
- persist configuration
- add three sequential white charge LEDs
- add 3 W warm-white muzzle emitter
- muzzle flash coincides with full-white ring blast
- portal color remains configurable
- acceleration curve remains configurable
- unwind direction remains configurable
- prefer native ESP32/I2S audio over DFPlayer
- network connectivity is optional
- normal prop operation must remain standalone

---

# 25. First Engineering Task

Do **not** immediately rewrite the original sketch.

First:

1. inspect the existing sketch
2. identify its behavioral responsibilities
3. identify hardware-coupled code versus pure logic
4. propose component/API boundaries
5. propose repository structure
6. identify ESP32-S3-specific migration issues
7. identify any behavior that cannot be preserved exactly and explain why

Produce an architectural proposal before implementing the migration.

The proposal should show where each major piece of the existing sketch will move.

---

# 26. Migration Success Criteria

The initial ESP32-S3 version should be considered successful when:

- it builds using ESP-IDF/CMake
- it runs on the target Lonely Binary ESP32-S3
- the original NeoPixel behavior is preserved
- all existing buttons behave as before
- existing settings persist across power cycles
- trigger behavior matches the original
- charge LEDs work
- muzzle flash works
- audio can be added or enabled without destabilizing the controller
- the firmware is modular enough to extend without returning to a monolithic sketch

The objective is to preserve the personality of the existing prop while replacing the prototype firmware architecture with a maintainable embedded application.
