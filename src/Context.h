#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h>
#include <macros.h>

class Scheduler;

namespace VMCBuddy {

struct _PulseChannel {
    uint8_t index;
    uint8_t interruptMode = DETACH_ISR;
    uint8_t pinMode = INPUT;
    uint8_t value = LOW;
    bool isDirty = false;
    unsigned long lastInterrupt = 0;
    uint16_t debounce = DEFAULT_PULSE_DEBOUNCE;
    const void* owner = nullptr;

    explicit _PulseChannel(uint8_t index = 0, uint8_t interruptMode = DETACH_ISR,
                           uint16_t debounce = DEFAULT_PULSE_DEBOUNCE)
        : index(index), interruptMode(interruptMode), debounce(debounce) {}
};

class Context {
private:
    Context();
    ~Context() = default;
    static Context* instance;

public:
    BoardConfig boardConfig;
    _PulseChannel pulseConfiguration[LEN_PIN_INTERRUPTS];
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    static Context* getInstance();

    void setScheduler(Scheduler* s) { _scheduler = s; }
    Scheduler* scheduler() { return _scheduler; }

    bool initialized = false;
    uint8_t shiftRegisterBits[LEN_SHIFT_REGISTER_BITS] = {};

private:
    Scheduler* _scheduler = nullptr;
};

inline Context::Context() : boardConfig(makeBoardConfig()) {
    for (uint8_t i = 0; i < boardConfig.numPulses; ++i)
        pulseConfiguration[i] = _PulseChannel(i, boardConfig.detachIsr,
                                              boardConfig.defaultPulseDebounce);
}

inline Context* Context::getInstance() {
    if (!instance) instance = new Context();
    return instance;
}

} // VMCBuddy

#endif //CONTEXT_H