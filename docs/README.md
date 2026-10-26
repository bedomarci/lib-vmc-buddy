# VMC Buddy Documentation

Documentation for the production board and its Arduino SDK. Hardware details are based on the [October 26, 2026 schematic set](./schematics/).

## Board documentation

- [Hardware overview](./hardware_interfaces.md) — board power, MCU mapping, pulse I/O, and interface index.
- [Jumper configuration](./jumper_configuration.md) — 2×15 through-hole jumper field and solder jumpers.
- [Configuration guide](./configuration_guide.md) — setup entry point and links to board-specific instructions.

## Interfaces

- [MDB](./MDB.md) — isolated MDB circuitry, connector, and operating modes.
- [Wiegand](./Wiegand.md) — D0/D1 input/output circuitry and connector.
- [RS-232](./RS232.md) — two serial channels and connector pinouts.
- [RS-485](./RS485.md) — half-duplex transceiver, bus connector, and direction control.

## SDK

- [SDK API reference](./sdk_api_reference.md) — implemented API, examples, pin map, and limitations.

## Schematic sheets

1. [Power](./schematics/SCH_Schematic3_1-power_2026-10-26.png)
2. [MCU](./schematics/SCH_Schematic3_2-mcu_2026-10-26.png)
3. [RS-232 and RS-485](./schematics/SCH_Schematic3_3-RS232-RS485-interface_2026-10-26.png)
4. [Pulse interfaces](./schematics/SCH_Schematic3_4-pulse-interfaces_2026-10-26.png)
5. [MDB](./schematics/SCH_Schematic3_5-MDB-interface_2026-10-26.png)
6. [Wiegand](./schematics/SCH_Schematic3_6-Wiegand-interface_2026-10-26.png)
7. [Miscellaneous interfaces and connectors](./schematics/SCH_Schematic3_7-misc-interfaces_2026-10-26.png)
