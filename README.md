# VMC Buddy

**ESP32 vending-machine interface hardware and Arduino SDK**

VMC Buddy is an ESP32-based interface board for vending machines and payment systems. Its PCB provides isolated MDB circuitry, Wiegand I/O, RS-485/RS-232 transceivers, optoisolated pulse I/O, onboard buttons, and LEDs. The current Arduino SDK implements pulse/LED/button functions only; serial and Wiegand protocol drivers are not present in `src/`.

---

## System Block Diagram

```mermaid
graph TB
    subgraph PSU["Power Supply"]
        DCIN["VCC-IN<br/>24-34V design input"] --> LM2596["LM2596S-5.0RG<br/>5V Buck"]
        LM2596 --> AMS1117["AMS1117-3.3<br/>3.3V LDO"]
        LM2596 --> B0505["B0505S-1WR2<br/>Isolated DC-DC"]
    end

    subgraph MCU["ESP32 module (schematic: WROOM-32E-N4)"]
        ESP32["ESP32<br/>240MHz / 3.3V"]
    end

    subgraph MDB["MDB Interface  "]
        MDB_UART["UART<br/>Optocoupler LTV-817S"]
        MDB_PWR["Isolated VCC5V / GND-MDB"]
        B0505 --> MDB_PWR
        ESP32 -. "UART net routing; no SDK driver" .-> MDB_UART
    end

    subgraph WIEGAND["Wiegand Interface"]
        WG_IN["Input: resistor conditioner<br/>+ 74HC14D Schmitt"]
        WG_OUT["Output: 2N7002<br/>Open-Drain MOSFET"]
        WG_IN --> ESP32
        ESP32 --> WG_OUT
    end

    subgraph RS485["RS-485"]
        SP3485["SP3485 Transceiver"]
        ESP32 -. "GPIO/UART; no SDK driver" .-> SP3485
    end

    subgraph RS232["RS-232"]
        SP3232["SP3232EEY 2-Ch"]
        ESP32 -. "UART; no SDK driver" .-> SP3232
    end

    subgraph PULSE["Pulse I/O"]
        S_IN["8x Opto-Input<br/>LTV-817S"]
        S_OUT["2x 74HC595D<br/>Shift Registers"]
        S_IN --> ESP32
        ESP32 --> S_OUT
    end

    AMS1117 --> ESP32
    AMS1117 --> RS485
    AMS1117 --> RS232
    AMS1117 --> PULSE
```

> **Galvanic isolation:** MDB signal circuitry uses B0505S-1WR2 (`MDB-VCC5V` / `GND-MDB`) and LTV-817S optocouplers. Connector and power routing are detailed in the [schematic-based hardware guide](./docs/hardware_interfaces.md).
>
> **Production board:** Hardware documentation follows the October 26, 2026 schematic set. See the [jumper configuration](./docs/jumper_configuration.md) and [hardware overview](./docs/hardware_interfaces.md) before wiring.

---

## Feature Matrix

| Interface | Hardware | Electrical details | SDK support |
|-----------|----------|-------------------|-------------|
| **MDB** | Solder-selectable master/slave routing | Isolated signal domain (B0505S + LTV-817S) | None |
| **Wiegand In** | D0/D1 input | 5V-side input, 74HC14D-conditioned TTL | None |
| **Wiegand Out** | D0/D1 open-drain | 2N7002, SM05T1G-ES, external pull-up | None |
| **RS-485** | Half-duplex SP3485 | 3.3V logic, PSM712, 120Ω termination/bias | None |
| **RS-232** | 2 TX/RX channels | SP3232EEY charge pump; PESD15VL2BT | None |
| **Pulse In** | 8 optoisolated channels | LTV-817S, separate `VCC-PULSE-*` nets | Interrupt reader (source limitation noted) |
| **Pulse Out** | 8 shift-register paths | 74HC595D + LTV-817S | Timed pulse API |
| **Onboard Keys** | 6-button analog ladder | 3.3V ADC | Internal polling/events |
| **Onboard LEDs** | 6 shift-register-driven LEDs | 3.3V logic | Blink API |

---

## Repository Structure

```
lib-vmc-buddy/
├── README.md                          ← This file
├── LICENSE                            ← GPL-3.0
├── platformio.ini                     ← Root project config
├── library.json                       ← PlatformIO library manifest
├── src/                               ← Library source
│   ├── VMCBuddy.h / VMCBuddy.cpp      ← Main public API
│   ├── Context.h                      ← Singleton runtime state
│   ├── EventHandler.h                 ← Event bus (subscribe/trigger)
│   ├── IO.h                           ← Pulse GPIO and shift-register I/O
│   ├── IPeripheral.h / ISerial.h       ← Scheduler and serial interfaces
│   ├── SerialPeripheral.h              ← HardwareSerial/SoftwareSerial adapter
│   ├── PulseReader.h                  ← ISR-based pulse input
│   ├── PulseWriter.h                  ← Pulse output generator
│   ├── PulseIDC.h                     ← IDC pulse-input connector mapping
│   ├── PulsePeripheral.h              ← Shared pulse mode/inhibit API
│   ├── PulseTerminal.h                ← 10-pin pulse terminal mapping
│   ├── Relay.h                         ← Two relay-named pulse outputs
│   ├── Led.h                          ← Onboard LED controller
│   ├── AnalogButtonReader.h           ← ADC button reader
│   ├── MDB*.h / RS*.h                  ← Serial interface layers
│   ├── CommandInterpreter.h            ← UART command parser
│   └── macros.h                       ← All pin definitions & constants
├── examples/
│   ├── pulse-out/                     ← Pulse output + LED toggle
│   ├── onboard-led/                   ← LED walk test
│   └── onboard-keys/                  ← Button event demo
└── docs/
    ├── README.md                       ← Documentation index
    ├── schematics/                     ← Production schematic exports (2026-10-26)
    ├── jumper_configuration.md         ← Through-hole and solder jumpers
    ├── hardware_interfaces.md          ← Board overview, pulse I/O, pin map
    ├── MDB.md / Wiegand.md             ← Interface details
    ├── RS232.md / RS485.md             ← Interface details
    ├── configuration_guide.md          ← Setup entry point
    └── sdk_api_reference.md            ← SDK API and limitations
```

The seven PNG schematic sheets in `docs/schematics/` are the production hardware drawings. They are linked from the [documentation index](./docs/README.md).

---

## Quickstart

### Prerequisites

- [PlatformIO IDE](https://platformio.org/) (VS Code extension recommended) or PlatformIO Core CLI
- ESP32 board supported by PlatformIO `esp32dev`; schematic identifies U29 as ESP32-WROOM-32E-N4, while `library.json` names ESP32-WROOM-32D-H4
- USB-to-UART cable for flashing and serial monitor

### Build & Upload

```bash
# Clone the repository
git clone https://github.com/bedomarci/lib-vmc-buddy
cd lib-vmc-buddy

# Build and upload to ESP32
pio run --target upload

# Open serial monitor
pio device monitor --baud 115200
```

### Minimal Example

```cpp
#include <Arduino.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseIDC.h>
#include <AnalogButtonReader.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseIDC pulseInputs(1, VMCBuddy::PulseIDC::Mode::Slave);
VMCBuddy::AnalogButtonReader buttons;
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    pulseInputs.setInterruptMode(1, FALLING); // local CH1; reader runs in this peripheral
    board.addPeripheral(&pulseInputs);
    board.addPeripheral(&buttons);
    board.addPeripheral(&leds);
    board.begin();
    leds.blink(0, 3, 200, 200);
}

void loop() {
    scheduler.execute();    // Run the cooperative scheduler
}
```

### Key APIs at a Glance

```cpp
VMCBuddy::PulseTerminal pulses(VMCBuddy::PulseTerminal::Mode::Master);
VMCBuddy::Led leds;
board.addPeripheral(&pulses); // before board.begin(); owns pulse channels 1–8
board.addPeripheral(&leds);
board.begin();
pulses.sendPulses(8, 3, 500, 200);          // local channel 8, 3 pulses
leds.blink(0, 5, 300, 100);                 // LED index 0, five blinks
```

Pulse interrupt events are handled by each active `PulsePeripheral` for its mapped input channels. `setOnEventCallback()` is declared but not implemented; use `EventHandler` directly.

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [arkhipenko/TaskScheduler](https://github.com/arkhipenko/TaskScheduler) | ^3.3.0 | Cooperative task scheduling |
| [thijse/ArduinoLog](https://github.com/thijse/ArduinoLog) | ^1.1.1 | Logging framework |

---

## Documentation

- [Documentation index](./docs/README.md) — Board and SDK documents
- [Hardware Interfaces](./docs/hardware_interfaces.md) — Board overview, pulse I/O, pin mapping
- [Jumper Configuration](./docs/jumper_configuration.md) — Through-hole and solder jumper maps
- [Configuration Guide](./docs/configuration_guide.md) — Setup entry point
- [SDK API Reference](./docs/sdk_api_reference.md) — Full class hierarchy, method signatures, code examples

---

## License

GPL-3.0 — see [LICENSE](./LICENSE).

## Author

Bedő Marci — bedomarci@gmail.com
