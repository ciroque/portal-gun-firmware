# ADR-0009: Defer the muzzle emitter and retain the selected driver for later

- Status: Accepted
- Date: 2026-10-25
- Supersedes: ADR-0006's muzzle implementation and switch preference only

## Context

Adding the muzzle emitter introduced current regulation, additional wiring,
thermal mounting, and bench validation. The owner chose to defer this subsystem
to simplify the initial build. This record preserves its selection history for
resumption; it is not a validated circuit design.

## Decision

Omit the muzzle emitter and its driver from the initial build. Retain the ring's
existing 240 ms full-white blast. Keep GPIO12 reserved for a future muzzle enable
signal, as allocated in ADR-0008. Do not connect the deferred hardware or include
its load in the initial build's power budget; revisit that budget when enabling it.

## Retained hardware and circuit choices

- The owner has both warm-white and cool-white CHANZON 3 W emitters. Cool white
  is preferred for the future muzzle flash, replacing the handoff's warm-white
  preference.
- Supplied listing summary: cool white 10000–15000 K, forward voltage 3.0–3.4 V,
  recommended current 600–700 mA. These are provisional: the supplied summary
  mixed product sources and was not a verified datasheet for the actual emitter.
- Target current: 600 mA from the regulated 5 V rail. Color temperature alone
  does not change the circuit if both emitters meet those electrical ratings.
- Intended timing remains on throughout BLAST (240 ms), off when DISCHARGE
  begins, coordinated with the ring by the controller.
- Adafruit #5648 is a suitable 3.3 V controlled low-side switch with sufficient
  current capacity, but provides no current regulation. It would need a separate
  current-limiting element. Its acquisition was not confirmed.
- A power resistor was considered as a simple alternative; it dissipates heat
  and allows current to vary with supply voltage and LED forward voltage.
  Constant-current regulation was preferred. No resistor circuit was finalized.
- MIC4801YM was investigated: 3.0–5.5 V input, up to 600 mA, logic enable, SOIC-8
  package. It was set aside because it requires SMT assembly and a custom board;
  the owner needs a compact, ready-made module.
- Selected prototype candidate: **Eletechsup LD06AJSA adjustable module**, based
  on CN5711. Advertised dimensions are **18 × 10.3 × 2.8 mm**, supply **2.8–6 V**,
  adjustable current **30–1500 mA**, with CE enable/PWM control. Select the onboard
  trimmer version and adjust its actual current with a meter before use.
- **The owner ordered three modules.** Delivery, exact supplied variants, and
  bench performance have not been confirmed. These are retained for later use.
- With GPIO12 controlling a compatible CE input, the module would provide both
  regulation and switching, eliminating the separate #5648 in this circuit.
  Confirm the delivered board's CE wiring, logic thresholds, and off-at-reset
  behavior before direct GPIO connection; provide an appropriate pull-down.

## Thermal implications retained for resumption

The CN5711 is a linear regulator, despite some listings calling the module a
"buck" converter. At 5 V and 600 mA, estimated driver heat during a flash is
`(5 V - Vf) × 0.6 A`, or **0.96–1.2 W**. LED electrical input is **1.8–2.04 W**;
some becomes light, so it is not all heat.

A 240 ms pulse gives roughly **0.23–0.29 J** of driver heat. At an illustrative
one shot per five seconds (4.8% duty cycle), average driver heat is approximately
**46–58 mW** and LED electrical input averages **86–98 mW**. This example is not
a firing-rate requirement or proof that heatsinking is unnecessary.

The proposed mounting approach was an aluminum spreader/bracket, an appropriate
thermal interface at the LED base (potentially via an LED star board), and an
electrically insulating thermal interface under the driver where needed. A shared
spreader may be practical if geometry permits. No heatsink dimensions or thermal
performance were validated. Printed plastic alone is not the LED's heat path.

The driver's thermal regulation can reduce LED current when hot. Before enabling
the subsystem, verify current and temperatures during repeated 240 ms pulses in
the actual enclosure, along with supply stability, switching, and safe mounting.

## Consequences

Initial assembly avoids muzzle-specific power wiring, current adjustment,
heatsinking, and testing. The emitter remains an optional later addition, with
three ordered driver modules available once received. Resumption requires the
electrical and thermal checks above rather than treating procurement as validation.

## References

- [Original handoff](../THE-NEW-HOTNESS.md), sections 14 and 16.
- [ADR-0006](0006-synchronized-lighting-and-muzzle.md) and
  [ADR-0008](0008-gpio-allocation.md).
- [Adafruit #5648 specifications](https://www.adafruit.com/product/5648).
- [Microchip MIC4801 datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/mic4801.pdf).
- [Eletechsup module listing](https://eletechsup.com/products/30-1500ma-adjustable-constant-current-led-driver-dc-3-3v-3-7v-4-2v-5v-pwm-control-board-buck-power-module).
- [CN5711 manufacturer information](https://consonance-elec.com/120.html).

Hardware availability, order quantity, emitter preference, and deferral were
confirmed by the owner in the project discussion on 2026-10-25.
