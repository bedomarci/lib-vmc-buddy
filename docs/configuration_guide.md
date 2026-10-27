# Configuration Guide

Use the dedicated [Jumper Configuration](./jumper_configuration.md) page for the complete production-board pin map and solder-bridge settings. It is the single source of truth for hardware jumper positions.

## Choose the interface

- [MDB](./MDB.md) — master/slave mode and isolated bus connections.
- [Wiegand](./Wiegand.md) — reader input or open-drain output.
- [RS-232](./RS232.md) — connector and UART channel mapping.
- [RS-485](./RS485.md) — connector, fixed termination/bias, and direction control.
- [Pulse I/O](./hardware_interfaces.md#pulse-io) — channel mapping and connector nets.

## Before changing hardware links

1. Disconnect board power.
2. Identify pin 1 from the production board marking before fitting a shunt.
3. Check the jumper guide for shared power nets and the MDB ground-isolation consequence.
4. Reapply power only after verifying the intended source and interface routing.

The schematics do not document factory shunt positions. The SDK provides RS-232, RS-485, and MDB interface classes; Wiegand still requires direct GPIO access. See the [SDK API Reference](./sdk_api_reference.md).
