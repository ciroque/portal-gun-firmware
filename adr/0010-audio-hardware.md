# ADR-0010: Use a MAX98357A amplifier with one 4 Ω speaker

- Status: Accepted
- Date: 2026-10-25
- Refines: ADR-0007's native I2S audio decision

## Context

The owner has Shutao miniature speakers rated 4 Ω / 3 W with JST-PH 2.0 mm
connectors. This project uses one speaker. Space is limited and the amplifier
should be a ready-made board powered from the regulated 5 V rail.

## Decision

Select the **Adafruit #3006 MAX98357A I2S mono Class D amplifier**. Its supply
range is 2.7–5.5 V, it accepts 3.3 V logic, supports a 4 Ω load, and requires
no MCLK. The advertised board dimensions are 19.4 × 17.8 × 3.0 mm; allow
additional clearance for the terminal block and wiring.

Use the existing ADR-0008 allocation:

| Connection | Source |
| --- | --- |
| VIN | Regulated 5 V rail |
| GND | Common ground |
| BCLK | GPIO15 |
| LRC / LRCLK | GPIO16 |
| DIN | GPIO17 |
| Speaker | Across the two amplifier output terminals |

Neither speaker lead connects to ground: the amplifier has a bridge-tied output.
Start playback at low volume and establish a clean maximum level on the actual
speaker. The advertised 3.2 W into 4 Ω at 5 V is specified at 10% THD, not a
clean-output target or permission to exceed the speaker's rating.

## Procurement and remaining integration

- Speaker: available, per owner; use one from the two-piece pack. Exact dimensions
  and connector pin markings have not been independently verified.
- Amplifier: **in the owner's cart, not ordered**. Purchasing is on hold until
  the full project parts list is established.
- Retain the speaker's JST connection. The owner has JST connectors available;
  exact mating parts, pin assignments, and wiring to the amplifier's screw
  terminals will be established later. No additional connector purchase is
  selected at this stage.
- Account for power/signal wiring, mounting clearance, and speaker mounting or
  a suitable acoustic chamber. No chamber design has been selected.

## Consequences

The amplifier and speaker selection is settled. Power capacity and wiring still
need review in the overall power budget. Asset format, storage, playback timing,
gain selection, and volume limits remain separate implementation decisions.

## References

- [Adafruit #3006 specifications](https://www.adafruit.com/product/3006).
- [ADR-0007](0007-native-audio-and-standalone-operation.md).
- [ADR-0008 GPIO allocation](0008-gpio-allocation.md).

Speaker specifications, availability, and amplifier cart status were supplied
by the owner in the project discussion on 2026-10-25.
