# KiCad subsystem wiring maps

These files are **functional wiring maps** created in KiCad's native schematic
file format. They capture current subsystem choices and GPIO assignments in
separate pages:

| Schematic | Subsystem |
| --- | --- |
| [controls.kicad_sch](controls.kicad_sch) | Four active-low buttons |
| [ring-lighting.kicad_sch](ring-lighting.kicad_sch) | ESP32-S3, AHCT buffer, and NeoPixel ring |
| [charge-indicators.kicad_sch](charge-indicators.kicad_sch) | Three charge indicator outputs; electrical interface marked TBD |
| [audio.kicad_sch](audio.kicad_sch) | I2S GPIOs, planned Adafruit #3006 amplifier, and one speaker |
| [power.kicad_sch](power.kicad_sch) | Planned battery, switch, regulator, and 5 V loads |
| [muzzle-deferred.kicad_sch](muzzle-deferred.kicad_sch) | Deferred muzzle hardware and reserved GPIO12 |

The pages use graphical blocks and lines, not KiCad electrical symbols, pins,
or electrical nets. They are therefore **not netlist-generating or ERC-checked
schematics**, and the connecting lines are illustrative. In particular, the
charge LED circuit is not designed, and the power system still needs
combined-load validation. Do not build from these pages as complete electrical
instructions.

The GPIO assignments and their status are authoritative in
[hardware-connections.md](../hardware-connections.md); refer to the linked ADRs
there for design decisions. These schematic drafts should be converted to
symbol-and-net schematics and checked in KiCad when the remaining electrical
details are settled.
