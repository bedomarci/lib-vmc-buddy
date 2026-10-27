#ifndef IO_H
#define IO_H

#include <Context.h>
#include <ArduinoLog.h>

namespace VMCBuddy {

/** GPIO and shift-register I/O for pulse channels. Onboard LED behavior
 * belongs to Led, not this low-level pulse I/O helper. */
class IO {
public:
    static void digitalWritePulse(uint8_t index, uint8_t state);
    static int digitalReadPulse(uint8_t index);
    static void pinModePulse(uint8_t index, uint8_t mode);
    static void setShiftRegisterBit(uint8_t index, uint8_t value);
    static void render();
};

inline void IO::pinModePulse(uint8_t index, uint8_t mode) {
    if (index >= NUM_PULSE) return;
    Context::getInstance()->pulseConfiguration[index].pinMode = mode;
    if (mode == INPUT)
        setShiftRegisterBit(Context::getInstance()->pulseOutPins[index], LOW);
}

inline void IO::digitalWritePulse(uint8_t index, uint8_t state) {
    if (index >= NUM_PULSE) return;
    if (Context::getInstance()->pulseConfiguration[index].pinMode != OUTPUT) {
        Log.errorln("Pulse #%i is INPUT — cannot digitalWrite.", index + 1);
        return;
    }
    setShiftRegisterBit(Context::getInstance()->pulseOutPins[index], state ? HIGH : LOW);
}

inline int IO::digitalReadPulse(uint8_t index) {
    if (index >= NUM_PULSE) return LOW;
    if (Context::getInstance()->pulseConfiguration[index].pinMode != INPUT) {
        Log.errorln("Pulse #%i is OUTPUT — cannot digitalRead.", index + 1);
        return LOW;
    }
    return digitalRead(Context::getInstance()->pulseInPins[index]);
}

inline void IO::setShiftRegisterBit(uint8_t index, uint8_t value) {
    if (index >= LEN_SHIFT_REGISTER_BITS) return;
    Context::getInstance()->shiftRegisterBits[index] = value ? HIGH : LOW;
    render();
}

inline void IO::render() {
    digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, LOW);
    for (unsigned char bit : Context::getInstance()->shiftRegisterBits) {
        digitalWrite(PIN_MCU_SHIFTREGISTER_DATA, bit ? HIGH : LOW);
        digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, HIGH);
        digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, LOW);
    }
    digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, HIGH);
}

} // namespace VMCBuddy
#endif // IO_H
