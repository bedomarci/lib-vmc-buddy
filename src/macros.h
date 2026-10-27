// ponytail: single pin-config source of truth.
//   * Macros for backward compat with existing code.
//   * BoardConfig struct for new forward-looking code.
//   * Both initialized from the same literal values here.
//   * Add #ifdef cascade when new board revisions ship.

#ifndef MACROS_H
#define MACROS_H

#include <Arduino.h>

// ——— GPIO pin assignments ———
#define PIN_MCU_SHIFTREGISTER_LATCH 21
#define PIN_MCU_SHIFTREGISTER_DATA  13
#define PIN_MCU_SHIFTREGISTER_CLOCK 5
#define PIN_MCU_ANALOG_BTNS         36
#define PIN_MCU_PULSE_IN1  27
#define PIN_MCU_PULSE_IN2  26
#define PIN_MCU_PULSE_IN3  25
#define PIN_MCU_PULSE_IN4  33
#define PIN_MCU_PULSE_IN5  32
#define PIN_MCU_PULSE_IN6  39
#define PIN_MCU_PULSE_IN7  14
#define PIN_MCU_PULSE_IN8  4

// ——— Shift-register bit assignments ———
#define NUM_PULSE 8
#define PIN_SR_PULSE_OUT1 15
#define PIN_SR_PULSE_OUT2 14
#define PIN_SR_PULSE_OUT3 13
#define PIN_SR_PULSE_OUT4 12
#define PIN_SR_PULSE_OUT5 11
#define PIN_SR_PULSE_OUT6 10
#define PIN_SR_PULSE_OUT7 9
#define PIN_SR_PULSE_OUT8 8

#define PIN_SR_RELAY1 PIN_SR_PULSE_OUT7
#define PIN_SR_RELAY2 PIN_SR_PULSE_OUT8

// ——— LED shift-register bits ———
#define NUM_LEDS 6
#define PIN_SR_LED1 7
#define PIN_SR_LED2 6
#define PIN_SR_LED3 5
#define PIN_SR_LED4 4
#define PIN_SR_LED5 3
#define PIN_SR_LED6 2

// ——— Analog buttons ———
#define TOLERANCE_ANALOG_BUTTONS_CHANGE 80
#define NUM_BUTTONS 6
#define VAL_ANALOG_BTN1 3150
#define VAL_ANALOG_BTN2 2880
#define VAL_ANALOG_BTN3 2560
#define VAL_ANALOG_BTN4 1820
#define VAL_ANALOG_BTN5 1120
#define VAL_ANALOG_BTN6 820
#define TIME_ANALOG_BUTTON_READ_INTERVAL 200

// ——— Constants ———
#define LEN_SHIFT_REGISTER_BITS 16
#define LEN_PIN_CONFIGURATION   12
#define LEN_PIN_INTERRUPTS      8
#define DETACH_ISR              0x00
#define DEFAULT_ISR_MODE        DETACH_ISR
#define DEFAULT_PULSE_DEBOUNCE  50

// ——— UART / serial GPIOs ———
#define PIN_UART0_TX 1
#define PIN_UART0_RX 3
#define PIN_UART1_TX 10
#define PIN_UART1_RX 9
#define PIN_UART2_TX 17
#define PIN_UART2_RX 16

// ——— RS-485 ———
#define PIN_RS485_TX_EN 15

// ——— RS-232 channel GPIOs ———
#define PIN_RS232_CH1_TX 19
#define PIN_RS232_CH1_RX 18
#define PIN_RS232_CH2_TX 17
#define PIN_RS232_CH2_RX 16

// ——— Wiegand GPIOs ———
#define PIN_WIEGAND_IN_D0  35
#define PIN_WIEGAND_IN_D1  34
#define PIN_WIEGAND_OUT_D0 23
#define PIN_WIEGAND_OUT_D1 22

// ——— BoardConfig struct (same values, structured access) ———
namespace VMCBuddy {

struct BoardConfig {
    // shift register
    uint8_t srLatch;
    uint8_t srData;
    uint8_t srClock;
    uint8_t srBitCount;

    // analog buttons
    uint8_t  btnAdc;
    uint8_t  numButtons;
    uint16_t btnReadIntervalMs;
    uint16_t btnTolerance;
    uint16_t btnThresholds[6];

    // pulse
    uint8_t numPulses;
    uint8_t pulseInGpio[8];
    uint8_t pulseOutSrBit[8];

    // leds
    uint8_t numLeds;
    uint8_t ledSrBit[6];

    // uart
    uint8_t uart0Tx, uart0Rx;
    uint8_t uart1Tx, uart1Rx;
    uint8_t uart2Tx, uart2Rx;

    // rs485
    uint8_t rs485TxEn;

    // rs232
    uint8_t rs232Ch1Tx, rs232Ch1Rx;
    uint8_t rs232Ch2Tx, rs232Ch2Rx;

    // wiegand
    uint8_t wiegandInD0, wiegandInD1;
    uint8_t wiegandOutD0, wiegandOutD1;

    // defaults
    uint8_t  detachIsr;
    uint16_t defaultPulseDebounce;
};

/*
 * Production board: October 26, 2026 — ESP32-WROOM-32E-N4.
 * All values verified against the 2026-10-26 schematic set.
 */
inline const BoardConfig& boardCfg() {
    static const BoardConfig cfg = {
        // shift register
        PIN_MCU_SHIFTREGISTER_LATCH,
        PIN_MCU_SHIFTREGISTER_DATA,
        PIN_MCU_SHIFTREGISTER_CLOCK,
        LEN_SHIFT_REGISTER_BITS,

        // analog buttons
        PIN_MCU_ANALOG_BTNS,
        NUM_BUTTONS,
        TIME_ANALOG_BUTTON_READ_INTERVAL,
        TOLERANCE_ANALOG_BUTTONS_CHANGE,
        {VAL_ANALOG_BTN1, VAL_ANALOG_BTN2, VAL_ANALOG_BTN3,
         VAL_ANALOG_BTN4, VAL_ANALOG_BTN5, VAL_ANALOG_BTN6},

        // pulse
        NUM_PULSE,
        {PIN_MCU_PULSE_IN1, PIN_MCU_PULSE_IN2, PIN_MCU_PULSE_IN3, PIN_MCU_PULSE_IN4,
         PIN_MCU_PULSE_IN5, PIN_MCU_PULSE_IN6, PIN_MCU_PULSE_IN7, PIN_MCU_PULSE_IN8},
        {PIN_SR_PULSE_OUT1, PIN_SR_PULSE_OUT2, PIN_SR_PULSE_OUT3, PIN_SR_PULSE_OUT4,
         PIN_SR_PULSE_OUT5, PIN_SR_PULSE_OUT6, PIN_SR_PULSE_OUT7, PIN_SR_PULSE_OUT8},

        // leds
        NUM_LEDS,
        {PIN_SR_LED1, PIN_SR_LED2, PIN_SR_LED3, PIN_SR_LED4, PIN_SR_LED5, PIN_SR_LED6},

        // uart
        PIN_UART0_TX, PIN_UART0_RX,
        PIN_UART1_TX, PIN_UART1_RX,
        PIN_UART2_TX, PIN_UART2_RX,

        // rs485
        PIN_RS485_TX_EN,

        // rs232
        PIN_RS232_CH1_TX, PIN_RS232_CH1_RX,
        PIN_RS232_CH2_TX, PIN_RS232_CH2_RX,

        // wiegand
        PIN_WIEGAND_IN_D0, PIN_WIEGAND_IN_D1,
        PIN_WIEGAND_OUT_D0, PIN_WIEGAND_OUT_D1,

        // defaults
        DETACH_ISR,
        DEFAULT_PULSE_DEBOUNCE,
    };
    return cfg;
}

} // namespace VMCBuddy

#endif // MACROS_H