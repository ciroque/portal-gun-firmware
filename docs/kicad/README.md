# KiCad subsystem schematics

Each subsystem has its own KiCad project. Open the `.kicad_pro` file in KiCad;
the schematic uses standard KiCad symbols and actual electrical nets.

| Project | Scope |
| --- | --- |
| [Control inputs](controls/controls.kicad_pro) | Four active-low switches and ESP32-S3 GPIO connections |
| [Ring lighting](ring-lighting/ring-lighting.kicad_pro) | GPIO8, AHCT buffer, bypass capacitor, and 12-pixel ring connections |
| [Charge indicators](charge-indicators/charge-indicators.kicad_pro) | GPIO9–11 interface; no LED circuit is drawn because its driver/current limiting is undecided |
| [Audio](audio/audio.kicad_pro) | ESP32-S3 I2S, Adafruit #3006 MAX98357A breakout connections, and bridge-tied speaker output |
| [Power](power/power.kicad_pro) | Battery connector, physical switch, regulator input/output, and 5 V/GND nets |

The muzzle emitter is deferred by [ADR-0009](../../adr/0009-defer-muzzle-emitter.md),
so it has no build schematic. These subsystem projects document the selected
connections; power capacity and final physical wiring still need the review in
[ADR-0011](../../adr/0011-power-budget.md).

The amplifier breakout is represented by its pin connector; the speaker itself
uses a speaker symbol and a two-wire connection. The module pin assignments
appear in connector net labels and project notes. The AHCT buffer is drawn with
the pin-compatible 74LS125 library symbol and labeled for the selected
74AHCT125-family function; its exact package/module remains to be settled.
