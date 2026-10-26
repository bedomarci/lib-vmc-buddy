# SDK Hardware Reference Manual: Wiegand Dual-Role Interface Module

## Table of Contents

1. [Document Overview & Key Features](https://www.google.com/search?q=%231-document-overview--key-features)
2. [Standard & Protocol Specifications](https://www.google.com/search?q=%232-standard--protocol-specifications)
3. [Hardware Architecture & Functional Description](https://www.google.com/search?q=%233-hardware-architecture--functional-description)
* [3.1 Input Stage Architecture](https://www.google.com/search?q=%2331-input-stage-architecture)
* [3.2 Output Stage Architecture](https://www.google.com/search?q=%2332-output-stage-architecture)
* [3.3 Dual-Role Switching Architecture](https://www.google.com/search?q=%2333-dual-role-switching-architecture)


4. [Connector Pinouts & Signal Definitions](https://www.google.com/search?q=%234-connector-pinouts--signal-definitions)
* [4.1 External Wiegand Interface Connector](https://www.google.com/search?q=%2341-external-wiegand-interface-connector)
* [4.2 MCU & Shift Register Signal Interface](https://www.google.com/search?q=%2342-mcu--shift-register-signal-interface)


5. [On-Board Hardware Configuration](https://www.google.com/search?q=%235-on-board-hardware-configuration)
* [5.1 Mode Selection Jumpers](https://www.google.com/search?q=%2351-mode-selection-jumpers)
* [5.2 Wiegand Input Mode Configuration Guide](https://www.google.com/search?q=%2352-wiegand-input-mode-configuration-guide)
* [5.3 Wiegand Output Mode Configuration Guide](https://www.google.com/search?q=%2353-wiegand-output-mode-configuration-guide)


6. [SDK & Software Integration Guidelines](https://www.google.com/search?q=%236-sdk--software-integration-guidelines)

---

## 1. Document Overview & Key Features

This document provides complete hardware reference specifications for the **Wiegand Dual-Role Interface Module** integrated into the VMC Buddy platform. The interface is designed to operate as either a **Wiegand Reader Input** (e.g., connecting external RFID readers, keypads, or biometric scanners) or a **Wiegand Device Output** (e.g., emulating a credential card to a master access control panel / VMC).

```
                      ┌────────────────────────────────────────┐
                      │    3-Pin Solder Jumpers (SJ_D0/SJ_D1)  │
                      └──────────────────┬─────────────────────┘
                                         │
                 ┌───────────────────────┴───────────────────────┐
                 ▼                                               ▼
     ┌──────────────────────┐                        ┌──────────────────────┐
     │     INPUT STAGE      │                        │     OUTPUT STAGE     │
     ├──────────────────────┤                        ├──────────────────────┤
     │ • TVS Protection     │                        │ • TVS Protection     │
     │ • 4.7kΩ Pull-up      │                        │ • 100Ω Series R      │
     │ • 74HC14D Schmitt-T. │                        │ • 2N7002 Open-Drain  │
     │ • 5V to 3.0V Divider │                        │ • Gate Pull-Down     │
     └───────────┬──────────┘                        └───────────▲──────────┘
                 │                                               │
                 ▼                                               │
     ┌──────────────────────┐                        ┌───────────┴──────────┐
     │  ESP32 Inputs        │                        │ ESP32 / SR Outputs   │
     │  (IO34 / IO35)       │                        │ (GPIO / 74HC595)     │
     └──────────────────────┘                        └──────────────────────┘

```

### Key Features

* **Selectable Dual-Role Design:** Configurable as Input or Output via 3-pin solder jumpers (`SJ_D0`, `SJ_D1`).
* **Industrial ESD & Surge Protection:** Dedicated TVS diode arrays (`SM05T1G-ES`, SOT-23) rated for $180\,\text{W}$ peak pulse power ($8/20\,\mu\text{s}$) placed directly at terminal entry points.
* **Hardware Noise Filtering:** Double-inverted Schmitt-trigger stages (`74HC14D`) eliminate contact bounce and edge jitter on long cable runs.
* **Safe 3.3V MCU Level Translation:** Integrated resistor dividers ($2.2\,\text{k}\Omega / 3.3\,\text{k}\Omega$) shift 5V logic down to $3.0\,\text{V}$ for direct connection to ESP32 input-only pins (`IO34`/`IO35`).
* **True Open-Drain Output:** Logic-level N-channel MOSFETs (`2N7002`) drive external 5V/12V pulled-up buses without voltage contention.
* **Boot-Safe Pull-Downs:** $10\,\text{k}\Omega$ Gate pull-down resistors prevent false card transmissions during MCU reset, power-up, or firmware update cycles.

---

## 2. Standard & Protocol Specifications

The module complies with standard SIA AC-01 (Wiegand Interface Standard) pulse timing and logic characteristics.

| Parameter | Symbol | Min | Typical | Max | Unit | Notes / Conditions |
| --- | --- | --- | --- | --- | --- | --- |
| **Operating Voltage (Bus)** | $V_{BUS}$ | 4.5 | 5.0 | 12.0 | $\text{V}$ | Pulled up externally or via R59/R60 |
| **Logic High Level** | $V_{IH}$ | 3.5 | 5.0 | $V_{BUS}$ | $\text{V}$ | Idle state on $D0$ and $D1$ lines |
| **Logic Low Level** | $V_{IL}$ | 0.0 | 0.2 | 0.8 | $\text{V}$ | Active pulse state |
| **Pulse Width** | $T_{PW}$ | 20 | 50 | 100 | $\mu\text{s}$ | Duration of active LOW pulse |
| **Pulse Interval** | $T_{PI}$ | 200 | 1000 | 2000 | $\mu\text{s}$ | Time between consecutive pulses |
| **Data Format** | — | 26 | 34 / 37 | 66 | Bits | Standard 26-bit / 34-bit / Custom formats |

* **Data 0 ($D0$):** Held HIGH ($5\,\text{V}$) during idle. Pulsed LOW to transmit a logical `0`.
* **Data 1 ($D1$):** Held HIGH ($5\,\text{V}$) during idle. Pulsed LOW to transmit a logical `1`.
* **Transmission Rule:** $D0$ and $D1$ are mutually exclusive and must never be driven LOW simultaneously.

---

## 3. Hardware Architecture & Functional Description

### 3.1 Input Stage Architecture

The input conditioning block processes signals coming from external readers before feeding them to the microcontroller.

```
External Line                      Series R          74HC14D (1A)      74HC14D (2Y)       ESP32 Input
WIEGAND_IN_D0 ───┬───────────────[ 220 Ω ]─────────┬───[Invert]───[Invert]───[ 2.2 kΩ ]───┬───► IO35
                 │                 (R61)          │                            (R8)      │
          [TVS: U33 Pin 2]                  [4.7 kΩ: R60]                              [3.3 kΩ: R7]
                 │                                │                                      │
                GND                             VCC5V                                   GND

```

1. **Transient Protection:** The external line connects directly to Pin 2 (or Pin 1) of the `U33` (`SM05T1G-ES`) TVS array. Electrostatic discharges (ESD) are clamped to ground before entering internal copper traces.
2. **Current Limiting & Pull-Up:** A $220\,\Omega$ resistor (`R61`/`R62`) limits current during transients. An internal $4.7\,\text{k}\Omega$ resistor (`R59`/`R60`) pulls the line to `VCC5V` for open-collector external readers. When the reader pulls the line to $0\,\text{V}$, the resulting voltage at the IC input is:

$$V_{IL(IC)} = 5\,\text{V} \times \frac{220\,\Omega}{4700\,\Omega + 220\,\Omega} \approx 0.22\,\text{V}$$



This value sits safely below the $1.5\,\text{V}$ lower threshold of the Schmitt trigger.
3. **Double Inversion (Schmitt-Trigger):** Passing through two consecutive channels of the `U22` (`74HC14D`) IC restores the original signal polarity while removing noise via its $0.8\,\text{V}$ typical hysteresis band.
4. **Logic Level Shifting:** A precision resistor divider ($2.2\,\text{k}\Omega / 3.3\,\text{k}\Omega$) scales the 5V CMOS output down to $3.0\,\text{V}$:

$$V_{OUT(MCU)} = 5\,\text{V} \times \frac{3.3\,\text{k}\Omega}{2.2\,\text{k}\Omega + 3.3\,\text{k}\Omega} = 3.0\,\text{V}$$



This ensures complete electrical protection for ESP32 input-only pins (`IO34` and `IO35`).

### 3.2 Output Stage Architecture

The output driver converts 3.3V logic signals into open-drain pulses compatible with any 5V or 12V host controller.

```
MCU / SR Output                      Gate R            2N7002 (Q1)       Series R         Output Line
WIEGAND-OUT-TTL-D0 ───[ 220 Ω ]───┬───(Gate)            (Drain) ──────[ 100 Ω ]───┬───► WIEGAND-OUT-D0
                       (R66)      │                       │             (R73)      │
                               [10 kΩ: R68]              GND                [TVS: U37 Pin 1]
                                  │                                                │
                                 GND                                              GND

```

1. **Gate Drive & Boot Safety:** A $220\,\Omega$ resistor (`R66`/`R67`) dampens high-frequency ringing on the MOSFET Gate. A $10\,\text{k}\Omega$ pull-down (`R68`/`R69`) holds the Gate at $0\,\text{V}$ during microcontroller initialization, preventing floating GPIOs from triggering false card reads.
2. **Open-Drain Transistor:** The `2N7002` N-channel MOSFET (`Q1`/`Q2`) acts as an open-drain switch.
* **Logic LOW Control (0V):** MOSFET is OFF; the Wiegand line floats high to $5\,\text{V}$ (or $12\,\text{V}$) via the host panel's pull-up.
* **Logic HIGH Control (3.3V):** MOSFET turns ON; the Wiegand line is actively pulled to $0\,\text{V}$.


3. **Short-Circuit Protection:** A $100\,\Omega$ series resistor (`R72`/`R73`) limits Drain current in case of accidental miswiring to power rails.
4. **Output TVS Protection:** The `U37` (`SM05T1G-ES`) TVS array clamps line transients directly at the output node.

### 3.3 Dual-Role Switching Architecture

To eliminate driver conflicts and save PCB space, the board utilizes 3-pin solder jumpers (`Solder Jumper 3p`) to route the external terminals (`WIEGAND_D0` and `WIEGAND_D1`) to either the input or output sub-circuit.

* **Pin 1:** Input Stage Path (`WIEGAND_IN_D0` / `D1`)
* **Pin 2 (Common):** External Terminal Connection (`WIEGAND_D0` / `D1`)
* **Pin 3:** Output Stage Path (`WIEGAND_OUT_D0` / `D1`)

---

## 4. Connector Pinouts & Signal Definitions

### 4.1 External Wiegand Interface Connector

The main external interface utilizes a screw terminal or header block (designated as `CON-WIEGAND`).

| Pin # | Signal Name | Type | Voltage Level | Description |
| --- | --- | --- | --- | --- |
| **1** | `VCC5V` / `VCC12V` | Power Out | 5.0V / 12.0V | Auxiliary power output for external card reader |
| **2** | `GND` | Power | 0V | System Ground Reference |
| **3** | `WIEGAND_D0` | I/O | 0V – 5V (12V) | Wiegand Data 0 Line (Configured via `SJ_D0`) |
| **4** | `WIEGAND_D1` | I/O | 0V – 5V (12V) | Wiegand Data 1 Line (Configured via `SJ_D1`) |

### 4.2 MCU & Shift Register Signal Interface

Internal routing between the Wiegand module, ESP32 microcontroller, and optional 74HC595 shift register expansion:

| Signal Name | Source / Destination | ESP32 Pin | Logic Direction | Functional Description |
| --- | --- | --- | --- | --- |
| `WIEGAND-IN-TTL-D0` | `U22` Pin 4 via Divider | `IO35` | Input Only | Conditioned Data 0 Input (3.0V active LOW) |
| `WIEGAND-IN-TTL-D1` | `U22` Pin 10 via Divider | `IO34` | Input Only | Conditioned Data 1 Input (3.0V active LOW) |
| `WIEGAND-OUT-TTL-D0` | GPIO / SR Output | `IO23` / SR Bit | Output | Data 0 Output Control (Active HIGH drives MOSFET) |
| `WIEGAND-OUT-TTL-D1` | GPIO / SR Output | `IO22` / SR Bit | Output | Data 1 Output Control (Active HIGH drives MOSFET) |

---

## 5. On-Board Hardware Configuration

### 5.1 Mode Selection Jumpers

The operating mode is established using two 3-pad SMD solder jumpers located on the top layer near the Wiegand circuit block:

* **`SJ_D0`:** Configures Data 0 routing.
* **`SJ_D1`:** Configures Data 1 routing.

> **CRITICAL WARNING:** Both jumpers (`SJ_D0` and `SJ_D1`) must ALWAYS be set to the same operational mode. Never solder Pads 1-2 on `SJ_D0` while soldering Pads 2-3 on `SJ_D1`.

```
    INPUT MODE (Pads 1-2)             OUTPUT MODE (Pads 2-3)
      ┌───┬───┬───┐                     ┌───┬───┬───┐
      │ 1 │ 2 │ 3 │                     │ 1 │ 2 │ 3 │
      └───┴───┴───┘                     └───┴───┴───┘
      [===SOLDER===]                        [===SOLDER===]
   IN ───┐   │   ┌─── OUT            IN ───┐   │   ┌─── OUT
         └───┴───┘                             └───┴───┘
           WIEGAND                               WIEGAND

```

### 5.2 Wiegand Input Mode Configuration Guide

Use this mode when connecting an external RFID reader to the VMC Buddy.

1. **Solder Jumper Settings:**
* Bridge **Pads 1 and 2** on `SJ_D0`.
* Bridge **Pads 1 and 2** on `SJ_D1`.


2. **Circuit Path Activated:** Connecting `WIEGAND_D0`/`D1` to `WIEGAND_IN_D0`/`D1` through `74HC14D` Schmitt triggers to ESP32 pins `IO35` / `IO34`.
3. **Outputs Handling:** Pins `IO22` and `IO23` can remain unconfigured or set as inputs.

### 5.3 Wiegand Output Mode Configuration Guide

Use this mode when the VMC Buddy acts as a credential device sending card data to an external VMC or access controller.

1. **Solder Jumper Settings:**
* Bridge **Pads 2 and 3** on `SJ_D0`.
* Bridge **Pads 2 and 3** on `SJ_D1`.


2. **Circuit Path Activated:** Connecting `WIEGAND_D0`/`D1` to `2N7002` Open-Drain MOSFETs (`Q1`/`Q2`).
3. **Inputs Handling:** ESP32 pins `IO34` and `IO35` remain idle at $3.0\,\text{V}$ HIGH level via internal pull-up network.

---

## 6. SDK & Software Integration Guidelines

### 6.1 Input Mode Driver Example (ESP-IDF / Arduino C++)

Because Wiegand input pulses are short ($50\,\mu\text{s}$), data acquisition must be handled via GPIO interrupts.

```cpp
#include <Arduino.h>

#define PIN_WIEGAND_IN_D0 35
#define PIN_WIEGAND_IN_D1 34

static volatile uint64_t rawCardData = 0;
static volatile uint8_t  bitCount = 0;
static volatile uint32_t lastPulseTime = 0;

void IRAM_ATTR isr_d0() {
    rawCardData = (rawCardData << 1); // Append logical 0
    bitCount++;
    lastPulseTime = millis();
}

void IRAM_ATTR isr_d1() {
    rawCardData = (rawCardData << 1) | 1; // Append logical 1
    bitCount++;
    lastPulseTime = millis();
}

void setupWiegandInput() {
    pinMode(PIN_WIEGAND_IN_D0, INPUT);
    pinMode(PIN_WIEGAND_IN_D1, INPUT);
    
    // Trigger on FALLING edge (active LOW pulses)
    attachInterrupt(digitalPinToInterrupt(PIN_WIEGAND_IN_D0), isr_d0, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_WIEGAND_IN_D1), isr_d1, FALLING);
}

void processWiegand() {
    // Timeout check: 25ms after last pulse indicates frame completion
    if (bitCount > 0 && (millis() - lastPulseTime > 25)) {
        noInterrupts();
        uint64_t data = rawCardData;
        uint8_t bits = bitCount;
        rawCardData = 0;
        bitCount = 0;
        interrupts();

        Serial.printf("Received Wiegand Frame: %d bits, Data: 0x%LLX\n", bits, data);
    }
}

```

### 6.2 Output Mode Driver via Shift Register (74HC595)

When controlling the Wiegand output MOSFETs via a shift register, use high-precision microsecond delays within a critical section to maintain pulse integrity.

```cpp
#include <Arduino.h>

#define SR_LATCH_PIN 5
// Assuming Bit 0 = WIEGAND_OUT_D0, Bit 1 = WIEGAND_OUT_D1
#define SR_BIT_D0 (1 << 0)
#define SR_BIT_D1 (1 << 1)

portMUX_TYPE wiegandMux = portMUX_INITIALIZER_UNLOCKED;

void writeShiftRegister(uint8_t data) {
    digitalWrite(SR_LATCH_PIN, LOW);
    shiftOut(13, 18, MSBFIRST, data); // Data Pin 13, Clock Pin 18
    digitalWrite(SR_LATCH_PIN, HIGH);
}

void sendWiegandBit(bool bitValue) {
    uint8_t pulseBit = bitValue ? SR_BIT_D1 : SR_BIT_D0;

    portENTER_CRITICAL(&wiegandMux);
    
    // Start pulse: Drive Gate HIGH -> MOSFET opens -> Bus drops to 0V
    writeShiftRegister(pulseBit);
    delayMicroseconds(50); // Standard 50us Wiegand pulse width
    
    // End pulse: Drive Gate LOW -> MOSFET closes -> Bus returns to 5V
    writeShiftRegister(0x00);
    
    portEXIT_CRITICAL(&wiegandMux);
    
    delayMicroseconds(1000); // 1ms inter-pulse space
}

void sendWiegandCard(uint32_t cardId, uint8_t bitLength) {
    for (int8_t i = bitLength - 1; i >= 0; i--) {
        bool bit = (cardId >> i) & 0x01;
        sendWiegandBit(bit);
    }
}

```