# Jumper Configuration

This guide documents the production board's 2x15 through-hole jumper field and solder jumpers. Pin numbers and nets follow the [October 26, 2026 schematics](./schematics/). Disconnect power before changing jumpers or solder links.

## 2x15 through-hole jumper field

The 30-pin field (U34 in the schematic) is arranged as 15 adjacent pin pairs. A shunt connects the two nets on its row.

| Pins | Connected nets | Purpose |
|---|---|---|
| 1–2 | `VCC-EXT` ↔ `VCC-IN` | Route external DC input to the board input rail |
| 3–4 | `VCC-PULSE-1-RAW` ↔ `VCC-IN` | Route board input rail to pulse supply group 1 |
| 5–6 | `VCC-PULSE-2-RAW` ↔ `VCC-IN` | Route board input rail to pulse supply group 2 |
| 7–8 | `VCC-MDB` ↔ `VCC-IN` | Route MDB supply to the board input rail |
| 9–10 | `GND-MDB` ↔ `GND` | Join isolated MDB ground to board ground |
| 11–12 | `VCC-RS232-1` ↔ `VCC5V` | Supply the RS-232 channel 1 connector rail |
| 13–14 | `VCC-RS232-2` ↔ `VCC5V` | Supply the RS-232 channel 2 connector rail |
| 15–16 | `VCC-PULSE-PULLUP` ↔ `VCC-IN` | Select board input rail for pulse pull-ups |
| 17–18 | `VCC-PULSE-PULLUP` ↔ `VCC5V` | Select 5 V rail for pulse pull-ups |
| 19–20 | `UART-RX2` ↔ `UART-RS232-RX2` | Route UART2 receive to RS-232 channel 2 |
| 21–22 | `UART-TX2` ↔ `UART-RS232-TX2` | Route UART2 transmit to RS-232 channel 2 |
| 23–24 | `UART-RX2` ↔ `UART-RS485-RX2` | Route UART2 receive to RS-485 |
| 25–26 | `UART-TX2` ↔ `UART-RS485-TX2` | Route UART2 transmit to RS-485 |
| 27–28 | `UART-RX2` ↔ `UART-MDB-RX` | Route UART2 receive to MDB |
| 29–30 | `UART-TX2` ↔ `UART-MDB-TX` | Route UART2 transmit to MDB |

### Routing notes

- Choose the power source for `VCC-IN` deliberately. Do not connect both `VCC-EXT` and `VCC-MDB` to it unless the power design specifically allows both sources to be tied together.
- Pins 15–16 and 17–18 select alternate sources for the same pulse pull-up net. Do not install both shunts; that would connect `VCC-IN` to `VCC5V` through the shared net.
- For UART2, populate only the receive/transmit pair for the intended interface. The header does not select the UART in firmware.
- Pins 9–10 directly join `GND-MDB` and `GND`; installing that shunt removes galvanic isolation between those grounds.
- The schematic defines the nets but does not specify factory shunt positions. Identify pin 1 from the board marking before fitting shunts.

## MDB mode solder jumpers

The MDB mode solder jumpers route the connector's receive and transmit lines. Set both links to the same mode.

| Mode | Receive-link pads | Transmit-link pads | Routing |
|---|---:|---:|---|
| Master | 1–2 | 1–2 | `MDB-MR` to master receive; `MDB-MT` to master transmit |
| Slave | 2–3 | 2–3 | `MDB-MR` to slave transmit; `MDB-MT` to slave receive |

The MDB receive-link solder jumper (U25) and transmit-link solder jumper (U26) each use a common middle pad. See the [MDB schematic](./schematics/SCH_Schematic3_5-MDB-interface_2026-10-26.png) and [MDB interface](./MDB.md).

## Wiegand mode solder jumpers

The two Wiegand mode solder jumpers route D0 and D1 between the input-conditioning circuits, external data lines, and output drivers. Set both to the same mode.

| Mode | D1 link | D0 link | Routing |
|---|---:|---:|---|
| Input | 1–2 | 1–2 | External D0/D1 to the Wiegand input circuits |
| Output | 2–3 | 2–3 | Output drivers to external D0/D1 |

The Wiegand D1 solder jumper (U39) and D0 solder jumper (U40) use pad 2 for the external data line. See the [Wiegand schematic](./schematics/SCH_Schematic3_6-Wiegand-interface_2026-10-26.png) and [Wiegand interface](./Wiegand.md).

## Interfaces without configuration jumpers

- RS-485 termination and bias components are fixed in the schematic; no termination-selection jumper is shown. See [RS-485](./RS485.md).
- The pulse schematic shows no mode-selection solder jumper. Pulse supply routing is handled by the through-hole field above.
- RS-232 channel 2 UART routing uses the through-hole field. Channel 1 is routed directly to its MCU nets.

See the [hardware overview](./hardware_interfaces.md) for connector and MCU details.
