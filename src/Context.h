#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h>
#include <macros.h>

class Scheduler;

namespace VMCBuddy {

class VMCBuddy;
class PulseChannel;

class Context {
private:
    Context();
    ~Context() = default;
    static Context* instance;

public:
    BoardConfig boardConfig;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    static Context* getInstance();

    void setScheduler(Scheduler* s) { _scheduler = s; }
    Scheduler* scheduler() { return _scheduler; }
    void setBoard(VMCBuddy* b) { _board = b; }
    VMCBuddy* board() { return _board; }

    bool initialized = false;
    uint8_t shiftRegisterBits[LEN_SHIFT_REGISTER_BITS] = {};

    PulseChannel* pulseChannels[NUM_PULSE] = {};

private:
    Scheduler* _scheduler = nullptr;
    VMCBuddy* _board = nullptr;
};

inline Context::Context() : boardConfig(makeBoardConfig()) {}

inline Context* Context::getInstance() {
    if (!instance) instance = new Context();
    return instance;
}

} // VMCBuddy

#endif //CONTEXT_H