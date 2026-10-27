# SDK API Reference

This reference reflects the implementation currently present in `src/`, not every circuit fitted to the PCB.

## Support status

| Hardware interface | Circuit on schematic | VMCBuddy SDK API |
|--------------------|---------------------|------------------|
| Pulse inputs | 8 optocoupled channels | `setPulseInterruptMode()` and event bus |
| Pulse outputs | 8 optocoupler-driven channels | `PulseWriter::sendPulses()` |
| Onboard LEDs | 6 LEDs via 74HC595D | `Led::set()` and `Led::blink()` |
| Analog buttons | 6-button resistor ladder | Internal polling; `EventHandler` events |
| MDB | Isolated transceiver/optocouplers | `MDB` and `MDBPeripheral` classes |
| Wiegand | Input buffer and MOSFET outputs | **No Wiegand API** |
| RS-485 | SP3485 transceiver | `RS485` class |
| RS-232 | Dual SP3232EEY transceiver | `RS232` class |
| Command interpreter | UART-based line parser | `CommandInterpreter` class |
| Pin configuration | Board-level GPIO mapping | `macros.h` definitions with `BoardConfig` view and runtime validation |

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

Configures pins listed in `Context::pinConfiguration`, then calls `begin()` on each peripheral registered with `addPeripheral()`. Register peripheral instances before calling `begin()`.

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
- `examples/onboard-keys` calls nonexistent `setOnChangeCallback()`.
- `Pulse::pinMode` is not initialized in the `Pulse` constructor, though `setPulseInterruptMode()` checks it. Interrupt setup may therefore depend on indeterminate state in the current source.
- Index methods do not consistently validate 0–7 / 0–5 bounds.
- Wiegand has no SDK API; the hardware circuit is available but must be driven through raw GPIOs.

## New interface classes

### `VMCBuddy::RS232`

| Method | Signature | Notes |
|---|---|---|
| Constructor | `RS232(uint8_t channel)` | Channel 1 uses UART1; channel 2 uses UART2 |
| `begin` | `begin(unsigned long baud=9600, uint32_t config=SERIAL_8N1)` | Configures HardwareSerial |
| `end` | `end()` | Releases the UART |
| `write` | `write(uint8_t c)` / `write(const uint8_t*, size_t)` | Blocking write |
| `read` | `int read()` | Returns next byte or -1 |
| `available` | `int available()` | Bytes in RX buffer |
| `flush` | `flush()` | Wait for TX completion |
| `stream` | `HardwareSerial* stream()` | Direct access to underlying HardwareSerial |

Channel 2 shares UART2 with RS-485 and MDB; the 2×15 through-hole jumper field selects the routing.

### `VMCBuddy::RS485`

| Method | Signature | Notes |
|---|---|---|
| Constructor | `RS485()` | Uses UART2, direction control on GPIO 15 |
| `begin` | `begin(unsigned long baud=9600, uint32_t config=SERIAL_8N1)` | Acquires UART2 bus; returns without configuring if conflict |
| `end` | `end()` | Releases UART2 bus |
| `write` | `write(uint8_t c)` / `write(const uint8_t*, size_t)` | Auto DE/RE toggling: HIGH → write → flush → LOW |
| `read` | `int read()` | Returns next byte or -1 |
| `available` | `int available()` | Bytes in RX buffer |
| `stream` | `HardwareSerial* stream()` | Direct access to underlying HardwareSerial |

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

Pin values are defined once in `macros.h`. `BoardConfig` provides structured access to the same values through `boardCfg()`.

| Field | Type | Description |
|---|---|---|
| `srLatch`, `srData`, `srClock` | `uint8_t` | Shift-register control GPIOs |
| `pulseInGpio[8]` | `uint8_t` | Pulse input GPIOs (index 0–7) |
| `pulseOutSrBit[8]` | `uint8_t` | Pulse output shift-register bit indices |
| `ledSrBit[6]` | `uint8_t` | LED shift-register bit indices |
| `uart0Tx`…`uart2Rx` | `uint8_t` | UART pin assignments |
| `rs485TxEn` | `uint8_t` | RS-485 direction-control GPIO |
| `rs232Ch1Tx`…`rs232Ch2Rx` | `uint8_t` | RS-232 channel GPIOs |
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
