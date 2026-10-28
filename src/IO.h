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
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return;
    context->pulseConfiguration[index].pinMode = mode;
    if (mode == INPUT)
        setShiftRegisterBit(context->boardConfig.pulseOutSrBit[index], LOW);
}

inline void IO::digitalWritePulse(uint8_t index, uint8_t state) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return;
    if (context->pulseConfiguration[index].pinMode != OUTPUT) {
        Log.errorln("Pulse #%i is INPUT — cannot digitalWrite.", index + 1);
        return;
    }
    setShiftRegisterBit(context->boardConfig.pulseOutSrBit[index], state ? HIGH : LOW);
}

inline int IO::digitalReadPulse(uint8_t index) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return LOW;
    if (context->pulseConfiguration[index].pinMode != INPUT) {
        Log.errorln("Pulse #%i is OUTPUT — cannot digitalRead.", index + 1);
        return LOW;
    }
    return digitalRead(context->boardConfig.pulseInGpio[index]);
}

inline void IO::setShiftRegisterBit(uint8_t index, uint8_t value) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.srBitCount) return;
    context->shiftRegisterBits[index] = value ? HIGH : LOW;
    render();
}

inline void IO::render() {
    const auto& context = *Context::getInstance();
    const auto& cfg = context.boardConfig;
    digitalWrite(cfg.srLatch, LOW);
    for (uint8_t i = 0; i < cfg.srBitCount; ++i) {
        digitalWrite(cfg.srData, context.shiftRegisterBits[i] ? HIGH : LOW);
        digitalWrite(cfg.srClock, HIGH);
        digitalWrite(cfg.srClock, LOW);
    }
    digitalWrite(cfg.srLatch, HIGH);
}

} // namespace VMCBuddy
#endif // IO_H
