# Firmware components

`platform` currently provides the portable board pin contract. Add `controller`,
`inputs`, `lighting`, `config`, and `audio` as their implementations arrive,
following [ADR-0016](../adr/0016-firmware-component-architecture.md). Do not add
empty components or tasks to stand in for future functionality.
