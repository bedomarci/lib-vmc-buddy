# SDK API Reference

This reference reflects the implementation currently present in `src/`, not every circuit fitted to the PCB.

## Support status

| Hardware interface | Circuit on schematic | VMCBuddy SDK API |
|--------------------|---------------------|------------------|
| Pulse inputs | 8 optocoupled channels | `setPulseInterruptMode()` and event bus |
| Pulse outputs | 8 optocoupler-driven channels | `PulseWriter::sendPulses()` |
| Pulse IDC connectors | Two connectors for input groups 1–3 and 4–6 | `PulseIDC` mapping adapter |
| Pulse terminal | 10-pin connector for channels 1–8 | `PulseTerminal` with selectable mode/inhibit |
| Onboard LEDs | 6 LEDs via 74HC595D | `Led::set()` and `Led::blink()` |
| Analog buttons | 6-button resistor ladder | Internal polling; `EventHandler` events |
| MDB | Isolated transceiver/optocouplers | `MDB` and `MDBPeripheral` classes |
| Wiegand | Input buffer and MOSFET outputs | **No Wiegand API** |
| RS-485 | SP3485 transceiver | `RS485` class |
| RS-232 | Dual SP3232EEY transceiver | `RS232` class |
| Command interpreter | UART-based line parser | `CommandInterpreter` class |
| Pin configuration | Board-level GPIO mapping | `Context::boardConfig`, initialized from the `macros.h` definitions |

Board GPIO/net details: [Hardware Interfaces](./hardware_interfaces.md). Jumper wiring: [Jumper Configuration](./jumper_configuration.md). Browse all board and SDK pages from the [documentation index](./README.md).

## Main class: `VMCBuddy::VMCBuddy`

Declared in `src/VMCBuddy.h`, implemented in `src/VMCBuddy.cpp`.

```cpp
VMCBuddy(Scheduler* scheduler, Print* serial = nullptr);
void begin();
void pinModePulse(uint8_t index, uint8_t mode);
void setPulseInterruptMode(uint8_t index, uint8_t mode,
                           uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);
void digitalWritePulsePin(uint8_t index, uint8_t value);
void addPeripheral(IPeripheral* peripheral);
void setOnEventCallback(std::function<void(uint8_t, int)> callback);
```

### Implemented calls

#### `VMCBuddy(Scheduler*, Print*)`

Requires a `TaskScheduler::Scheduler*`. Optional `Print*` supplies ArduinoLog output; null selects `Serial`. Keep scheduler alive for the board lifetime and call `scheduler.execute()` in `loop()`.

#### `begin()`

Configures shift-register, ADC, and pulse GPIOs from `Context::boardConfig`, then calls `begin()` on each peripheral registered with `addPeripheral()`. Register peripheral instances before calling `begin()`.

#### `pinModePulse(uint8_t index, uint8_t mode)`

Uses **zero-based** pulse index 0–7. `INPUT` clears the corresponding shift-register output bit; `OUTPUT` marks the channel for output use. It does not call Arduino `pinMode()` on an external pulse connector.

#### `setPulseInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce)`

Intended for zero-based input indexes 0–7. Supported Arduino interrupt modes are `CHANGE`, `RISING`, or `FALLING`; `DETACH_ISR` (0) leaves channel disabled. Default debounce is 50 ms. Call before `begin()`.

#### `addPeripheral(IPeripheral*)`

Adds the task to `TaskScheduler` and the board's peripheral list. `VMCBuddy::begin()` then starts it. Peripheral objects must outlive the board.

## Counted pulse output: `PulseWriter`

`PulseWriter` is a reusable `IPeripheral`; its constructor does not register itself with a scheduler. Add it to the board, select pulse output mode, then call:

```cpp
bool sendPulses(uint8_t pulseIndex, uint16_t count,
                uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                uint16_t startDelayMs = 0);
```

Pulse index is zero-based (0–7). The task emits exactly `count` HIGH pulses, using the requested pulse and pause durations, then disables itself. Calling `sendPulses()` while active restarts the sequence with new parameters.

## Onboard LED controller: `Led`

`Led` owns control of the six active-low onboard LEDs and is an `IPeripheral`. Add one controller instance to the board. Both methods accept only zero-based LED indexes 0–5:

```cpp
bool set(uint8_t ledIndex, uint8_t state);
bool blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
           uint16_t offTimeMs, uint16_t startDelayMs = 0);
```

`blink()` reuses `PulseWriter`'s restartable phase task; it returns false for an invalid LED index or pulse parameters.

## Shared pulse direction: `PulsePeripheral`

`PulsePeripheral` is the common `PulseWriter`-derived peripheral base for `PulseIDC` and `PulseTerminal`. Master drives data channels and reads INHIBIT; Slave reads data channels and drives INHIBIT. It provides `setMode()`, `read()`, `setInterruptMode()`, inherited `sendPulses()`, and inhibit selection/control. `setInhibitChannel(1..channelCount)` selects the inhibit channel; zero disables it. `PulseIDC` fixes INHIBIT to local channel 3; `PulseTerminal` accepts any local channel 1–8. Select mode and configure input interrupts before `VMCBuddy::begin()`.

Master output uses the separate `PULSE-OUT-TTL-CHx` shift-register path. The production schematic does not electrically route that output back to either input connector.

## Pulse IDC connectors

`PulseIDC` represents either 2×5 connector and exposes local roles CH1, CH2, and INHIBIT. Connector 1 maps local roles to board channels 1–3; connector 2 maps to board channels 4–6. INHIBIT defaults to local CH3 and may be disabled with `setInhibitChannel(0)`. Select connector and mode at construction:

```cpp
VMCBuddy::PulseIDC idc1(1, VMCBuddy::PulseIDC::Mode::Slave);
VMCBuddy::PulseIDC idc2(2, VMCBuddy::PulseIDC::Mode::Master);
```

| Connector | Local CH1 | Local CH2 | Local INHIBIT |
|---|---|---|---|
| 1 | Input: pin 8, `PULSE-EXT-CH1`; Master output: `PULSE-OUT-TTL-CH1` | Input: pin 6, `PULSE-EXT-CH2`; Master output: `PULSE-OUT-TTL-CH2` | Input: pin 4, `PULSE-EXT-CH3`; Master output: `PULSE-OUT-TTL-CH3` |
| 2 | Input: pin 8, `PULSE-EXT-CH4`; Master output: `PULSE-OUT-TTL-CH4` | Input: pin 6, `PULSE-EXT-CH5`; Master output: `PULSE-OUT-TTL-CH5` | Input: pin 4, `PULSE-EXT-CH6`; Master output: `PULSE-OUT-TTL-CH6` |

The connector's physical `PULSE-EXT` nets are optoisolated inputs. In Master mode, writes use the corresponding separate shift-register `PULSE-OUT-TTL` path; the schematic does not wire that output path back onto the IDC contact.

| Method | Behavior |
|---|---|
| `setMode(Mode)` | Select `Master` (output) or `Slave` (input) before begin |
| `read(localChannel)` | Master: reads local INHIBIT; Slave: reads CH1/CH2 |
| `setInterruptMode(localChannel, mode, debounceMs)` | Configure an input channel before board begin |
| `sendPulses(localChannel, count, pulseMs, pauseMs)` | Master: counted output on CH1/CH2; Slave: output on INHIBIT |
| `setInhibit(active)` | Drive local INHIBIT in Slave mode |
| `readInhibit()` | Read local INHIBIT in Master mode |
| `pulseIndex(localChannel)` / `gpio(localChannel)` / `idcPin(localChannel)` | Resolve local role to board channel, GPIO, or IDC contact |

The constructor is `PulseIDC(uint8_t connectorNumber, Mode mode=Mode::Slave)`. Register each `PulseIDC` plus one shared `PulseReader` with VMCBuddy.

## Pulse terminal connector

`PulseTerminal` uses the same `PulsePeripheral` Master/Slave and INHIBIT API for the 10-pin terminal. Local channels 1–8 map directly to board channels 1–8. Select one optional local channel (1–8) as INHIBIT in the constructor; zero disables inhibit. Master drives all other channels and reads INHIBIT; Slave reads the other channels and drives INHIBIT:

```cpp
VMCBuddy::PulseTerminal terminal(VMCBuddy::PulsePeripheral::Mode::Slave, 8);
board.addPeripheral(&terminal);
```

Terminal pins 1–8 carry external pulse channels 1–8; pin 9 is `VCC-IN`, pin 10 is `GND`. Constructor: `PulseTerminal(Mode mode=Mode::Slave, uint8_t inhibitChannel=0)`. `read()`, `setInterruptMode()`, `sendPulses()`, `readInhibit()`, and `setInhibit()` follow the selected direction. Master outputs use separate `PULSE-OUT-TTL-CHx` nets; the schematic does not connect them back to this terminal.

### Declared, currently not implemented

`setOnEventCallback()` is declared in `VMCBuddy.h` but has no definition in `VMCBuddy.cpp`. `EventHandler` below is the implemented event registration path.

## Pulse and LED example

```cpp
#include <Arduino.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseWriter.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseWriter pulses;
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    board.pinModePulse(6, OUTPUT);
    board.addPeripheral(&pulses);
    board.addPeripheral(&leds);
    board.begin();
    pulses.sendPulses(6, 3, 250, 250);
    leds.blink(0, 3, 200, 200);
}

void loop() {
    scheduler.execute();
}
```

## Event system: `EventHandler`

Defined in `src/EventHandler.h`. It uses raw function pointers and three integer event parameters.

```cpp
enum class Event { BUTTON_PRESS, BUTTON_RELEASE, PULSE_INT };
using EventCallback = void(*)(int, int, int);

static void subscribe(Event event, EventCallback handler);
static void unsubscribe(EventCallback handler);
static void trigger(Event event, int param1 = 0,
                    int param2 = 0, int param3 = 0);
```

Event payloads generated by current code:

| Event | `param1` | `param2` | `param3` |
|-------|----------|----------|----------|
| `BUTTON_PRESS` | Button index 0–5 | 0 | 0 |
| `BUTTON_RELEASE` | -1 | 0 | 0 |
| `PULSE_INT` | Pulse index 0–7 | sampled GPIO value | configured interrupt mode |

Example:

```cpp
#include <EventHandler.h>

void onPulse(int index, int value, int mode) {
    Serial.printf("Pulse %d value=%d mode=%d\n", index, value, mode);
}

void setup() {
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::PULSE_INT, onPulse);
}
```

Subscribe to `BUTTON_PRESS` or `BUTTON_RELEASE` in the same way. The callback is invoked from the scheduler task, not directly by the pulse ISR.

## Internal implementation

| Class | Source | Role |
|-------|--------|------|
| `Context` | `Context.h/.cpp` | Singleton pulse state, pin configuration, and shift-register shadow |
| `IO` | `IO.h` | Pulse GPIO/shift-register I/O and 16-bit bit-banged render |
| `PulseReader` | `PulseReader.h` | GPIO ISRs, debounce, deferred `PULSE_INT` dispatch |
| `PulseWriter` | `PulseWriter.h` | Registered reusable Task for counted pulse output |
| `Led` | `Led.h` | Onboard LED controller; blink reuses PulseWriter phases |
| `AnalogButtonReader` | `AnalogButtonReader.h` | GPIO36 ADC sampling every 200 ms |

Shift-register control nets from `macros.h` and the MCU sheet: DATA GPIO13, CLOCK GPIO5, LATCH GPIO21. The firmware writes 16 bits via `digitalWrite()`; no SPI peripheral is used.

## Pin and constant map

### Direct ESP32 GPIOs used by SDK

| Macro | GPIO | Use |
|-------|------|-----|
| `PIN_MCU_SHIFTREGISTER_LATCH` | 21 | Shift-register latch |
| `PIN_MCU_SHIFTREGISTER_DATA` | 13 | Shift-register serial data |
| `PIN_MCU_SHIFTREGISTER_CLOCK` | 5 | Shift-register clock |
| `PIN_MCU_ANALOG_BTNS` | 36 | Analog button input |
| `PIN_MCU_PULSE_IN1` … `PIN_MCU_PULSE_IN8` | 27, 26, 25, 33, 32, 39, 14, 4 | Pulse inputs 1–8 |

The SDK does **not** currently configure MDB, Wiegand, RS-485, or RS-232 GPIO/UARTs. Consult schematic net mappings and board revision before writing direct Arduino code for those interfaces.

### Shift-register indexes (`macros.h`)

| Index | Function |
|-------|----------|
| 15–8 | Pulse outputs 1–8 |
| 7–2 | LEDs 1–6 |
| 1–0 | Unassigned in the SDK macros |

The six button thresholds are 3150, 2880, 2560, 1820, 1120, and 820 ADC units in order BTN1–BTN6. `TOLERANCE_ANALOG_BUTTONS_CHANGE` is 80.

## Current source limitations

- `setOnEventCallback()` lacks a definition.
- Index methods do not consistently validate 0–7 / 0–5 bounds.
- Wiegand has no SDK API; the hardware circuit is available but must be driven through raw GPIOs.

## New interface classes

### `VMCBuddy::SerialPeripheral`

Shared serial implementation base between `ISerial` and the RS-232/RS-485 concrete classes.

| Method | Signature | Notes |
|---|---|---|
| `setSerial` | `void setSerial(Stream* serial)` | Inject `HardwareSerial`, `SoftwareSerial`, or another Arduino `Stream` |
| `stream` | `Stream* stream()` | Returns the active/injected stream |
| `begin` | `bool begin()` | If no stream was injected, selects and starts the board's default HardwareSerial |
| `end` | `void end()` | Stops only the HardwareSerial instance opened by the default path |

Injected streams are caller-owned and must be configured before `begin()`. Call `setSerial(nullptr)` to return to the board's default hardware serial. MDB stays on its GPIO bit-bang transport because Arduino `Stream` cannot carry MDB's 9th bit.

### `VMCBuddy::RS232`

| Method | Signature | Notes |
|---|---|---|
| Constructor | `RS232(uint8_t channel)` | Channel 1 uses UART1; channel 2 uses UART2 |
| `begin` | `begin()` / `begin(unsigned long baud, uint32_t config=SERIAL_8N1)` | Injected Stream is already configured; default path starts HardwareSerial |
| `end` | `end()` | Releases the UART |
| `write` | `write(uint8_t c)` / `write(const uint8_t*, size_t)` | Blocking write |
| `read` | `int read()` | Returns next byte or -1 |
| `available` | `int available()` | Bytes in RX buffer |
| `flush` | `flush()` | Wait for TX completion |
| `setSerial` | `setSerial(Stream*)` | Inherited from `SerialPeripheral`; supports SoftwareSerial |

Channel 2 shares UART2 with RS-485 and MDB; the 2×15 through-hole jumper field selects the routing.

### `VMCBuddy::RS485`

| Method | Signature | Notes |
|---|---|---|
| Constructor | `RS485()` | Uses UART2, direction control on GPIO 15 |
| `begin` | `begin()` / `begin(unsigned long baud, uint32_t config=SERIAL_8N1)` | Acquires UART2 route; false on conflict |
| `end` | `end()` | Releases UART2 bus |
| `write` | `write(uint8_t c)` / `write(const uint8_t*, size_t)` | Auto DE/RE toggling: HIGH → write → flush → LOW |
| `read` | `int read()` | Returns next byte or -1 |
| `available` | `int available()` | Bytes in RX buffer |
| `setSerial` | `setSerial(Stream*)` | Inherited from `SerialPeripheral`; supports SoftwareSerial |

### `VMCBuddy::MDB` (L1–L2)

GPIO bit-bang 9-bit MDB/ICP driver at 9600 baud. Uses ESP32 UART2 pins (GPIO16/17) directly — no HardwareSerial dependency. TX is blocking (~1.1 ms per byte). RX uses a falling-edge ISR + FreeRTOS queue with `ets_delay_us` timing (1 ms ISR hold per byte).

| Method | Signature | Notes |
|---|---|---|
| Constructor | `MDB(uint8_t peripheralAddr=0x01)` | Sets the peripheral address |
| `begin` | `bool begin()` | Configures GPIO16/17, installs ISR on GPIO16 falling edge, creates RX queue |
| `end` | `void end()` | Disables ISR, deletes queue, releases GPIOs |
| `read9` | `int read9()` | Returns next 9-bit word from RX queue; modeBit stored in bit 8 |
| `write9` | `void write9(uint8_t data, bool modeBit)` | Blocking bit-bang TX: start bit → 9 data bits LSB-first → stop bit |
| `readPacket` | `Packet* readPacket(uint32_t timeoutMs=5)` | Frame assembly with address/cmd detection and checksum validation |
| `sendPacket` | `void sendPacket(uint8_t cmd, const uint8_t* data, uint8_t len)` | Blocking packet TX: address → cmd → data bytes → checksum, 5 ms gap |
| `isRunning` | `bool isRunning()` | Whether ISR is active |
| `address` | `uint8_t address()` | Peripheral address |

**Timing:** 104 µs per bit (9600 baud). ISR hold time: ~1.1 ms per byte (start + 9 data + stop). TX hold time: same. MDB claims the UART2/GPIO16/17 resource slot shared with RS-485 and RS-232 channel 2. Their `begin()` methods return false if another interface already owns it. Set the 2×15 jumper field for exactly one physical path. **Do not run WiFi/BT on core 1** — bit-bang timing is sensitive to ISR preemption.

`Packet` fields: `addr`, `cmd`, `data[64]`, `dataLen`, `valid`.

### `VMCBuddy::MDBPeripheral` (L3)

MDB peripheral state machine. Inherits `MDB`, which supplies `IPeripheral` and `Task` behavior.

| Method | Signature | Notes |
|---|---|---|
| Constructor | `MDBPeripheral(uint8_t addr, Print* log=nullptr)` | Task that polls every 1 ms |
| `begin` | `bool begin()` | Starts bit-bang transport and enables task |
| `end` | `void end()` | Stops task and releases shared route |
| `onPoll` | `void onPoll(PollHandler)` | Handler returns activity byte for POLL response |
| `onReset` | `void onReset(ResetHandler)` | Called on RESET command (0x08) |
| `onSetup` | `void onSetup(SetupHandler)` | Called on SETUP command (0x0F) |
| `onVend` | `void onVend(VendHandler)` | Called on VEND command (0x0A) |
| `onExpansion` | `void onExpansion(ExpansionHandler)` | Called on EXPANSION command (0x11) |

### `VMCBuddy::CommandInterpreter`

UART line-based command dispatcher. Inherits `IPeripheral` and `Task`.

| Method | Signature | Notes |
|---|---|---|
| Constructor | `CommandInterpreter(Stream*)` | Sets up 10 ms polling task |
| `begin` | `bool begin()` | Enables the task; called by VMCBuddy |
| `end` | `void end()` | Disables the task |
| `on` | `bool on(const char* cmd, CmdHandler)` | Register a command handler |
| `off` | `void off(const char* cmd)` | Remove a command handler |

Command format: `CMD [ARG1] [ARG2] ...` (newline-terminated). Commands are matched case-insensitively. Unmatched commands echo `UNKNOWN: <line>` back to the stream.

### `VMCBuddy::BoardConfig`

`Context` owns the single active `BoardConfig` instance as `Context::getInstance()->boardConfig`. `macros.h` holds the pin literals and constructs that structured configuration once; runtime code reads the Context instance.

| Field | Type | Description |
|---|---|---|
| `srLatch`, `srData`, `srClock` | `uint8_t` | Shift-register control GPIOs |
| `pulseInGpio[8]` | `uint8_t` | Pulse input GPIOs (index 0–7) |
| `pulseOutSrBit[8]` | `uint8_t` | Pulse output shift-register bit indices |
| `ledSrBit[6]` | `uint8_t` | LED shift-register bit indices |
| `uart0Tx`…`uart2Rx` | `uint8_t` | UART pin assignments |
| `rs485TxEn` | `uint8_t` | RS-485 direction-control GPIO |
| `rs232Ch1Tx`, `rs232Ch1Rx` | `uint8_t` | RS-232 channel 1 GPIOs; channel 2 reuses UART2 pins |
| `wiegandInD0`…`wiegandOutD1` | `uint8_t` | Wiegand input/output GPIOs |
| `btnAdc`, `btnThresholds[6]` | `uint8_t`/`uint16_t` | Analog button configuration |

Call `VMCBuddy::validateBoardConfig()` at runtime to check GPIO uniqueness and shift-register bit assignments. `UARTBus` prevents simultaneous claims on shared UART/GPIO routes.

### `VMCBuddy::UARTBus`

UART resource allocation tracker.

| Method | Signature | Notes |
|---|---|---|
| `acquire` | `static bool acquire(uint8_t uartNum, Owner owner)` | Claim UART/GPIO route. Returns false if already owned |
| `release` | `static void release(uint8_t uartNum)` | Release a UART |
| `owner` | `static Owner owner(uint8_t uartNum)` | Current owner (NONE/MDB/RS485/RS232_CH1/RS232_CH2/CMD) |

RS-232, RS-485, and MDB `begin()` automatically claim their route and fail if another interface owns it.

## Build configuration

Root `platformio.ini` uses `espressif32`, `esp32dev`, Arduino, TaskScheduler `^3.3.0`, and ArduinoLog `^1.1.1`. Build/upload with:

```sh
pio run
pio run --target upload
pio device monitor --baud 115200
```
