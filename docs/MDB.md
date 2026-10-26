# SDK Hardware Reference Manual: MDB/ICP Dual-Role Interface Module

## Table of Contents

* [1. Document Overview & Key Features](https://www.google.com/search?q=%231-document-overview--key-features)
* [2. Standard & Protocol Specifications](https://www.google.com/search?q=%232-standard--protocol-specifications)
* [3. Hardware Architecture & Functional Description](https://www.google.com/search?q=%233-hardware-architecture--functional-description)
* [3.1 Power Supply & Galvanic Isolation](https://www.google.com/search?q=%2331-power-supply--galvanic-isolation)
* [3.2 Transmitter (TX) Circuitry](https://www.google.com/search?q=%2332-transmitter-tx-circuitry)
* [3.3 Receiver (RX) Dual-Opto Architecture](https://www.google.com/search?q=%2333-receiver-rx-dual-opto-architecture)


* [4. Connector Pinouts & Signal Definitions](https://www.google.com/search?q=%234-connector-pinouts--signal-definitions)
* [4.1 MDB Main Connector (CON-MDB)](https://www.google.com/search?q=%2341-mdb-main-connector-con-mdb)
* [4.2 Auxiliary DC Power Input (DC1 / CN1)](https://www.google.com/search?q=%2342-auxiliary-dc-power-input-dc1--cn1)


* [5. On-Board Hardware Configuration](https://www.google.com/search?q=%235-on-board-hardware-configuration)
* [5.1 Main Selector Matrix (U28)](https://www.google.com/search?q=%2351-main-selector-matrix-u28)
* [5.2 Mode Routing Jumpers (U25, U26, U34)](https://www.google.com/search?q=%2352-mode-routing-jumpers-u25-u26-u34)
* [5.3 Master Mode Configuration Guide](https://www.google.com/search?q=%2353-master-mode-configuration-guide)
* [5.4 Slave Mode Configuration Guide](https://www.google.com/search?q=%2354-slave-mode-configuration-guide)


* [6. SDK & Software Integration Guidelines](https://www.google.com/search?q=%236-sdk--software-integration-guidelines)

---

## 1. Document Overview & Key Features

The **MDB/ICP Dual-Role Interface Module** provides a fully isolated hardware bridge between an ESP32 microcontroller (3.3V UART) and standard NAMA MDB/ICP vending machine networks. The module is hardware-configurable to function either as a **Vending Machine Controller (VMC / Master)** or as a **Peripheral Device (Bill Validator, Cashless Reader, Coin Mechanism / Slave)** using on-board jumper routing.

### Key Hardware Features

* **Dual-Role Capabilities:** Master (VMC) and Slave (Peripheral) operation on a single PCB.
* **Galvanic Isolation:** Complete isolation between local system ground (`GND`) and MDB bus ground (`GND-MDB`) using LTV-817/PC817 optocouplers and an isolated B0505S-1W DC-DC converter[cite: 11].
* **Zero-Idle LED Wear Architecture:** Dual-opto RX topology guarantees that optocoupler IR LEDs remain unpowered during UART Idle states in both Master and Slave modes, eliminating LED degradation.
* **Native UART Logic:** Direct non-inverted 3.3V UART signal output (`Idle = HIGH`), requiring no software signal inversion.
* **Teensy/ESP32 Safe:** Low GPIO current loading (< 5 mA per TX line using $470\,\Omega$ current-limiting resistors).

---

## 2. Standard & Protocol Specifications

The module complies with the **NAMA MDB / ICP (Multi-Drop Bus / Internal Communication Protocol) Specification v4.3**[cite: 10].

| Parameter | Specification | Notes |
| --- | --- | --- |
| **Bus Voltage Range** | 20V to 42.5V DC (34V DC Nominal)[cite: 7] | Sourced from MDB bus (Slave mode) or external jack (Master mode)[cite: 9] |
| **Baud Rate** | 9600 Baud ($\pm 1\%$)[cite: 10] | Fixed hardware rate[cite: 10] |
| **Data Format** | 9-Bit UART (1 Start, 8 Data, 1 Mode, 1 Stop)[cite: 10] | Bit 9 = Mode Bit (1: Address/Attention, 0: Data)[cite: 10] |
| **Master TX Signaling** | Active +5V Drive[cite: 10] | Active drive (+5V = Space/0, 0V = Mark/1)[cite: 10] |
| **Slave TX Signaling** | Open-Collector Pull-Down[cite: 10] | Active pull-down (GND-MDB = Space/0, Float = Mark/1)[cite: 10] |

---

## 3. Hardware Architecture & Functional Description

```
                  +-------------------------------------------------+
                  |              GALVANIC ISOLATION                 |
                  |                                                 |
[ESP32 MCU] <---> | [Optocouplers U20/U36] <---> [MDB-TX] <-------> | <---> [CON-MDB]
  (3.3V UART)     | [Optocouplers U32/U35] <---> [MDB-RX] <-------> |       (34V Bus)
                  |                                                 |
                  | [B0505S Isolated DC-DC] ---> MDB-VCC5V (5V Iso) |
                  +-------------------------------------------------+

```

### 3.1 Power Supply & Galvanic Isolation

* **Local Power Generation:** Main board power ($24\text{V} - 34\text{V}$) is stepped down via an LM2596 high-voltage buck converter to produce local system `VCC5V` and an AMS1117-3.3 LDO to supply `VCC3V`[cite: 7].
* **Isolated Logic Supply:** A 1W isolated DC-DC converter module (`U28`, B0505S-1W) converts local `VCC5V` into a galvanically isolated 5V rail (`MDB-VCC5V`) relative to `GND-MDB`[cite: 11]. This rail powers Master-mode active line drivers and biases receiver optocouplers[cite: 11].

### 3.2 Transmitter (TX) Circuitry

The transmitter stage utilizes two independent optocoupler pathways to handle the distinct electrical demands of Master and Slave modes:

* **Master Mode TX (`U36`):** Configured as a High-Side Emitter-Follower. The collector (Pin 4) is tied to `MDB-VCC5V` and the emitter (Pin 3) drives `MDB-TX-MASTER` with a $1\text{ k}\Omega$ pull-down resistor (`R29`) to `GND-MDB`.
* *Idle State (`UART-MDB-TX` = 3.3V):* `U36` LED is OFF $\rightarrow$ Output is pulled to 0V (`GND-MDB`).
* *Active State (`UART-MDB-TX` = 0V):* `U36` LED turns ON $\rightarrow$ Drives output to +5V (`MDB-VCC5V`).


* **Slave Mode TX (`U20`):** Configured as a standard Open-Collector Low-Side Driver. The emitter (Pin 3) is connected to `GND-MDB` and the collector (Pin 4) connects to `MDB-TX-SLAVE`.
* *Idle State (`UART-MDB-TX` = 3.3V):* `U20` LED is OFF $\rightarrow$ Output floats (Open-Collector).
* *Active State (`UART-MDB-TX` = 0V):* `U20` LED turns ON $\rightarrow$ Pulls output to `GND-MDB`.



### 3.3 Receiver (RX) Dual-Opto Architecture

To avoid software signal inversion and ensure longevity, two dedicated receiver optocouplers are deployed:

* **Master Mode RX (`U32`):** Anode (Pin 1) connected to `MDB-VCC5V`; Cathode (Pin 2) connected via $470\,\Omega$ (`R31`) to `MDB-RX-MASTER` (connected to MDB Pin 4 / MR)[cite: 11]. Captures active open-collector pull-downs from Slave peripherals[cite: 11].
* **Slave Mode RX (`U35`):** Anode (Pin 1) connected via $470\,\Omega$ (`R42`) to `MDB-RX-SLAVE` (connected to MDB Pin 5 / MT); Cathode (Pin 2) connected to `GND-MDB`[cite: 11]. Captures active +5V pulses driven by the VMC[cite: 11].
* **Combined Output:** The collector outputs of `U32` and `U35` are wired in parallel to the `UART-MDB-RX` line with a $10\text{ k}\Omega$ pull-up (`R27`) to `VCC3V`.

---

## 4. Connector Pinouts & Signal Definitions

### 4.1 MDB Main Connector (`CON-MDB`)

* **Connector Type:** 6-Pin Molex Mini-Fit Jr. Header (4.20mm pitch)[cite: 5].

```
  +---+---+---+
  | 1 | 2 | 3 |
  +---+---+---+
  | 4 | 5 | 6 |
  +---+---+---+
 (Front View)

```

| Pin # | Signal Name | Master Mode Role | Slave Mode Role | Connected Net |
| --- | --- | --- | --- | --- |
| **1** | **+34V DC** | Power Output (24V–34V DC to bus)[cite: 5] | Power Input (Nourishes board)[cite: 5] | `VCC-MDB`[cite: 5] |
| **2** | **Power GND** | Power Return Ground[cite: 5] | Power Return Ground[cite: 5] | `GND-MDB`[cite: 11] |
| **3** | **Shield** | No Connection / Shield[cite: 5] | No Connection / Shield[cite: 5] | N/C[cite: 5] |
| **4** | **MDB-MR** | Master Receive Input (Signal IN)[cite: 5] | Slave Transmit Output (Signal OUT)[cite: 5] | Via `U25` Jumper[cite: 10] |
| **5** | **MDB-MT** | Master Transmit Output (Signal OUT)[cite: 5] | Slave Receive Input (Signal IN)[cite: 5] | Via `U26` Jumper[cite: 10] |
| **6** | **Comms Common** | Isolated Signal Reference Ground[cite: 5] | Isolated Signal Reference Ground[cite: 5] | `GND-MDB`[cite: 11] |

### 4.2 Auxiliary DC Power Input (`DC1` / `CN1`)

* **Connector Type:** Standard 2.1mm / 5.5mm DC Barrel Jack (`DC1`) or 2-pin terminal block (`CN1`)[cite: 6].
* **Usage:** Required **ONLY** in Master Mode to provide bus and system power ($24\text{V} - 34\text{V}$ DC)[cite: 6, 9].

| Pin # | Signal Name | Description | Connected Net |
| --- | --- | --- | --- |
| **1** | **DC IN (+)** | External DC Input ($+24\text{V}$ to $+34\text{V}$ DC)[cite: 6] | `VCC-EXT-RAW` (via SS34 diode `D22` $\rightarrow$ `VCC-EXT`)[cite: 6] |
| **2 / 3** | **DC GND (-)** | System Power Ground[cite: 6] | Local `GND`[cite: 6] |

---

## 5. On-Board Hardware Configuration

### 5.1 Main Selector Matrix (`U28`)

Header `U28` (2x15 Pin 2.54mm Header) manages power pathing, ground ties, and UART routing[cite: 9].

```
       LEFT (Odd)            RIGHT (Even)
   +---------------------------------------+
 1 | VCC-EXT          || VCC-IN            | 2
 3 | VCC-PULSE-1-RAW  || VCC-IN            | 4
 5 | VCC-PULSE-2-RAW  || VCC-IN            | 6
 7 | VCC-MDB          || VCC-IN            | 8
 9 | GND-MDB          || GND               | 10
11 | VCC-RS232-1      || GND               | 12
...| ...              || ...               | ...
27 | UART-RX2         || UART-MDB-RX       | 28
29 | UART-TX2         || UART-MDB-TX       | 30
   +---------------------------------------+

```

### 5.2 Mode Routing Jumpers (`U25`, `U26`, `U34`)

* **`U25` (3-pin Solder Jumper):** Routes MDB Receive Line (MR)[cite: 10].
* **`U26` (3-pin Solder Jumper):** Routes MDB Transmit Line (MT)[cite: 10].
* **`U34` (2-pin / 3-pin Jumper):** Enables/Disables Master TX $1\text{ k}\Omega$ pull-down path[cite: 11].

---

### 5.3 Master Mode Configuration Guide (VMC Mode)

Configure the board to operate as a Master Controller supplying power and controlling peripherals[cite: 5, 9]:

1. **Power Configuration (`U28`):**
* Place jumper cap on **Pins 1–2** (`VCC-EXT` $\rightarrow$ `VCC-IN`)[cite: 9].
* Place jumper cap on **Pins 7–8** (`VCC-MDB` $\rightarrow$ `VCC-IN`)[cite: 9].
* Place jumper cap on **Pins 9–10** (`GND-MDB` $\rightarrow$ `GND`)[cite: 9].


2. **UART Routing (`U28`):**
* Place jumper cap on **Pins 27–28** (`UART-RX2` $\leftrightarrow$ `UART-MDB-RX`)[cite: 8, 9].
* Place jumper cap on **Pins 29–30** (`UART-TX2` $\leftrightarrow$ `UART-MDB-TX`)[cite: 8, 9].


3. **Signal Jumpers:**
* **`U25`:** Solder pads **1–2** (`MDB-MR` $\leftrightarrow$ `MDB-RX-MASTER`)[cite: 10].
* **`U26`:** Solder pads **1–2** (`MDB-MT` $\leftrightarrow$ `MDB-TX-MASTER`)[cite: 10].
* **`U34`:** Closed / Jumpered ON (connects `MDB-TX-MASTER` to $1\text{ k}\Omega$ `R29` pull-down)[cite: 11].


4. **Power Input:** Plug 24V–34V DC power supply into DC Jack `DC1`[cite: 6].

---

### 5.4 Slave Mode Configuration Guide (Peripheral Mode)

Configure the board to operate as a Peripheral powered and controlled by an external VMC[cite: 5, 9]:

1. **Power Configuration (`U28`):**
* Place jumper cap on **Pins 7–8** (`VCC-MDB` $\rightarrow$ `VCC-IN`)[cite: 9].
* Place jumper cap on **Pins 9–10** (`GND-MDB` $\rightarrow$ `GND`)[cite: 9].
* *Leave Pins 1–2 OPEN (DC Jack disconnected)*[cite: 8, 9].


2. **UART Routing (`U28`):**
* Place jumper cap on **Pins 27–28** (`UART-RX2` $\leftrightarrow$ `UART-MDB-RX`)[cite: 8, 9].
* Place jumper cap on **Pins 29–30** (`UART-TX2` $\leftrightarrow$ `UART-MDB-TX`)[cite: 8, 9].


3. **Signal Jumpers:**
* **`U25`:** Solder pads **2–3** (`MDB-MR` $\leftrightarrow$ `MDB-TX-SLAVE`)[cite: 10].
* **`U26`:** Solder pads **2–3** (`MDB-MT` $\leftrightarrow$ `MDB-RX-SLAVE`)[cite: 10].
* **`U34`:** Open / Unjumpered OFF[cite: 11].


4. **Power Input:** Powered directly through MDB connector Pin 1 (`CON-MDB`)[cite: 5, 9].

---

## 6. SDK & Software Integration Guidelines

### ESP32 Hardware UART Settings

The hardware design provides standard un-inverted UART logic signals to the ESP32 GPIO pins. Software-level signal inversion is **not required**.

* **Baud Rate:** 9600 bps[cite: 10]
* **Data Bits:** 8 bits[cite: 10]
* **Parity:** None (Handled via 9th bit / Parity Bit mechanism in ESP-IDF)[cite: 10]
* **Stop Bits:** 1 bit[cite: 10]
* **Signal Inversion:** Disabled (`invert_rx = false`, `invert_tx = false`)

### ESP-IDF Configuration Example (C++)

```cpp
#include "driver/uart.h"

#define MDB_UART_NUM      UART_NUM_2
#define MDB_TX_PIN        GPIO_NUM_17
#define MDB_RX_PIN        GPIO_NUM_16

void mdb_hardware_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install driver and configure GPIOs
    ESP_ERROR_CHECK(uart_driver_install(MDB_UART_NUM, 256, 256, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(MDB_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(MDB_UART_NUM, MDB_TX_PIN, MDB_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Disable signal inversion (Dual-Opto HW handles un-inverted logic natively)
    ESP_ERROR_CHECK(uart_set_line_inverse(MDB_UART_NUM, UART_SIGNAL_INV_DISABLE));
}

```