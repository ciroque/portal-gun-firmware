# ADR-0011: Budget the initial build's power before confirming the supply

- Status: Proposed; component retention pending load validation
- Date: 2026-10-25
- Refines: ADR-0005

## Context

The planned supply is an Adafruit #1781 protected 2200 mAh single-cell battery,
hardware switch, and Pololu #4085 S13V20F5 regulated 5 V bus. The initial load
includes ring lighting, controller, charge indicators, and the amplifier selected
in ADR-0010. The muzzle subsystem is deferred by ADR-0009.

## Proposed budget

These are design allowances, not measured consumption or guaranteed maxima.

| Load | Allowance referred to the 5 V supply | Basis |
| --- | --- | --- |
| 12-pixel ring | 0.72 A | 12 × 60 mA planning rule at full-white blast |
| ESP32-S3 development board | 0.30 A | Provisional standalone-operation allowance; measure actual board |
| MAX98357A audio | 0.70 A | 3 W / (5 V × assumed 90% efficiency), rounded up |
| Three charge LEDs | 0.03 A | Up to 10 mA each; included in board input budget here for simplicity |
| Buffer, inputs, miscellaneous | 0.02 A | Provisional allowance |
| **Coincident load** | **1.77 A / 8.85 W** | White blast plus loud playback |

Audio draw depends on the waveform and volume; this is not a bound for arbitrary
clipped signals. The ESP32 allowance needs review if networking is enabled.
The speaker's output power must not be counted again as a separate supply load.

## Supply assessment

Pololu's "2 A" label is not a guarantee across the battery range: continuous
capacity depends on input voltage and temperature. Verify the manufacturer's
current curve and actual performance at low battery voltage before accepting
this supply. The short blast can ease average heating but does not establish
transient capability.

At an assumed 85% conversion efficiency, the coincident budget implies battery
current of approximately **2.81 A at 3.7 V**, or **3.25 A at 3.2 V**. Allow for
wiring losses and regulator overhead in measurements.

Adafruit specifies a 4.4 A peak capability for #1781 but recommends sustained
current below about 1 A; an acceptable pulse duration is not established here.
At 1 A battery current and 85% efficiency, the available 5 V current is only
about **0.63 A at 3.7 V** or **0.54 A at 3.2 V**. Sustained loud audio and bright
lighting therefore cannot be approved solely from the peak rating.

## Proposal and verification

Retain the supply as a candidate for a prototype, not a confirmed unrestricted
design. Measure idle/charge average consumption and coincident white-blast/audio
peaks, voltage sag, battery current, and regulator temperature across the usable
battery range in the intended enclosure. Establish volume limits using actual
assets. If loads exceed battery or regulator capability, choose whether to cap
audio output or revise the supply; do not silently reduce the reference blast.

Keep the planned ring bulk capacitor (~1000 µF, ≥10 V) and local bypassing.
Capacitance handles brief transients; it cannot supply a 240 ms power deficit.
Distribute peripheral power from the regulated bus with common grounds rather
than routing amplifier/ring current through controller GPIO or breadboard paths.
Final wire/connector ratings and USB/external-supply interaction require a wiring
review. No purchase or replacement supply is selected by this proposal.

## References

- [Pololu #4085 specifications and output-current curve](https://www.pololu.com/product/4085).
- [Adafruit #1781 battery current guidance](https://www.adafruit.com/product/1781).
- [Adafruit NeoPixel power planning](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels).
- [Adafruit #3006 amplifier specifications](https://www.adafruit.com/product/3006).

Sources checked 2026-10-25. Conversion and audio efficiencies above are explicit
calculation assumptions, not measured efficiencies of the finished prop.
