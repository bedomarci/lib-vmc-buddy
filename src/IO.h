#ifndef IO_H
#define IO_H

#include <Context.h>

namespace VMCBuddy {

class IO {
public:
    static void setShiftRegisterBit(uint8_t index, uint8_t value);
    static void render();
};

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