---
plan name: sdk-expansion
plan description: SDK interface and commands
plan status: done
---

## Idea
Expand the VMC Buddy SDK with one pin-config source, a scheduler-owned peripheral model, RS-232/RS-485 objects, GPIO bit-bang MDB/ICP support, an MDB peripheral layer, and a UART command interpreter. Reuse installed libraries where they fit; keep transport glue specific to this board small.

## Implementation
- Keep pin literals in `src/macros.h`; store the single structured `BoardConfig` instance in `Context` and use it throughout the SDK.
- Add `IPeripheral` and `ISerial`; register peripheral Tasks through `VMCBuddy::addPeripheral()`.
- Add `SerialPeripheral` with `setSerial(Stream*)`, default HardwareSerial selection, and injected-stream ownership rules.
- Implement `RS232` and `RS485` wrappers with UART route ownership checks.
- Implement MDB/ICP 9-bit bit-bang on GPIO16/17, packet framing, and MDB peripheral callbacks.
- Add cashless, bill-validator, and coin-changer MDB use-case base classes.
- Add the UART `CommandInterpreter` as a scheduler-managed peripheral.
- Make `PulseReader`, `AnalogButtonReader`, `PulseWriter`, and `Led` schedulable peripherals.
- Add a shared mode-aware `PulsePeripheral` base, plus `PulseIDC` and `PulseTerminal` mappings with selectable INHIBIT channel.
- Keep pulse output and onboard LED controls separate; `Led::blink()` reuses the counted pulse task phases.
- Document the APIs and verify board examples build.

## Required Specs
<!-- SPECS_START -->
- board-pin-config
<!-- SPECS_END -->
