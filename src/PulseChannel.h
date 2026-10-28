#ifndef PULSE_CHANNEL_H
#define PULSE_CHANNEL_H

#include <Context.h>
#include <EventHandler.h>
#include <IO.h>

namespace VMCBuddy {

class PulseChannel {
public:
    explicit PulseChannel(uint8_t index, Context* ctx)
        : _ctx(ctx)
        , _state(&ctx->pulseConfiguration[index])
        , _index(index) {}

    // ---- input mode ----
    int read() const;
    bool setInterruptMode(uint8_t mode, uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);
    void attachInterrupt();
    void disableInterrupt();
    void checkDirty();

    // ---- output mode ----
    void write(uint8_t val);
    bool startPulseTrain(uint16_t count, uint16_t pulseLengthMs,
                         uint16_t pauseLengthMs, uint16_t startDelayMs = 0);
    void updateWriter();
    bool writerBusy() const { return _phase != Phase::Idle; }
    uint16_t writerDelayMs() const;
    void stopWriter();

    // ---- common ----
    bool isValid() const { return _index < _ctx->boardConfig.numPulses; }
    uint8_t index() const { return _index; }
    uint8_t pinMode() const { return _state->pinMode; }
    void setPinMode(uint8_t mode) { IO::pinModePulse(_index, mode); }
    bool isInput() const { return _state->pinMode == INPUT; }
    const void* owner() const { return _state->owner; }
    void setOwner(const void* o) { _state->owner = o; }

private:
    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };

    Context* _ctx;
    _PulseChannel* _state;
    uint8_t _index;

    Phase _phase = Phase::Idle;
    uint16_t _remaining = 0;
    uint16_t _pulseLengthMs = 0;
    uint16_t _pauseLengthMs = 0;
    uint32_t _nextTransition = 0;

    static void IRAM_ATTR handleInterruptArg(void* arg);
    static void IRAM_ATTR handleInterrupts(uint8_t pulseIndex);
};

inline int PulseChannel::read() const {
    if (!isValid() || _state->pinMode != INPUT ||
        _state->owner == nullptr) return -1;
    return IO::digitalReadPulse(_index);
}

inline bool PulseChannel::setInterruptMode(uint8_t mode, uint16_t debounce) {
    if (!isValid() || _state->pinMode != INPUT) return false;
    if (mode != DETACH_ISR && mode != CHANGE && mode != RISING && mode != FALLING)
        return false;
    _state->interruptMode = mode;
    _state->debounce = debounce;
    return true;
}

inline void PulseChannel::attachInterrupt() {
    if (!isValid() || _state->pinMode != INPUT ||
        _state->interruptMode == DETACH_ISR) return;
    _state->isDirty = false;
    attachInterruptArg(_ctx->boardConfig.pulseInGpio[_index],
                       PulseChannel::handleInterruptArg,
                       reinterpret_cast<void*>(static_cast<uintptr_t>(_index + 1)),
                       _state->interruptMode);
}

inline void PulseChannel::disableInterrupt() {
    if (isValid())
        detachInterrupt(_ctx->boardConfig.pulseInGpio[_index]);
}

inline void PulseChannel::checkDirty() {
    if (!isValid() || !_state->isDirty || _state->pinMode != INPUT) return;
    EventHandler::trigger(Event::PULSE_INT, _state->index, _state->value, _state->interruptMode);
    _state->isDirty = false;
}

inline void IRAM_ATTR PulseChannel::handleInterruptArg(void* arg) {
    const uintptr_t encodedIndex = reinterpret_cast<uintptr_t>(arg);
    auto* ctx = Context::getInstance();
    if (encodedIndex == 0 || encodedIndex > ctx->boardConfig.numPulses) return;
    handleInterrupts(static_cast<uint8_t>(encodedIndex - 1));
}

inline void IRAM_ATTR PulseChannel::handleInterrupts(const uint8_t pulseIndex) {
    auto* ctx = Context::getInstance();
    auto& pulse = ctx->pulseConfiguration[pulseIndex];
    if (pulse.pinMode != INPUT) return;
    const unsigned long now = millis();
    if (now - pulse.lastInterrupt > pulse.debounce) {
        pulse.value = digitalRead(ctx->boardConfig.pulseInGpio[pulseIndex]);
        pulse.isDirty = true;
        pulse.lastInterrupt = now;
    }
}

inline void PulseChannel::write(uint8_t val) {
    IO::digitalWritePulse(_index, val);
}

inline bool PulseChannel::startPulseTrain(uint16_t count, uint16_t pulseLengthMs,
                                          uint16_t pauseLengthMs, uint16_t startDelayMs) {
    if (!isValid() || _state->pinMode != OUTPUT || count == 0 || pulseLengthMs == 0)
        return false;
    if (_phase == Phase::Pulse) IO::digitalWritePulse(_index, LOW);
    _remaining = count;
    _pulseLengthMs = pulseLengthMs;
    _pauseLengthMs = pauseLengthMs;
    _phase = Phase::Start;
    _nextTransition = millis() + startDelayMs;
    return true;
}

inline void PulseChannel::updateWriter() {
    if (_phase == Phase::Idle ||
        static_cast<int32_t>(millis() - _nextTransition) < 0) return;
    const uint32_t now = millis();
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        IO::digitalWritePulse(_index, HIGH);
        _phase = Phase::Pulse;
        _nextTransition = now + _pulseLengthMs;
        break;
    case Phase::Pulse:
        IO::digitalWritePulse(_index, LOW);
        if (--_remaining == 0) _phase = Phase::Idle;
        else {
            _phase = Phase::Pause;
            _nextTransition = now + _pauseLengthMs;
        }
        break;
    case Phase::Idle:
        break;
    }
}

inline uint16_t PulseChannel::writerDelayMs() const {
    if (_phase == Phase::Idle) return 1;
    const int32_t remaining = static_cast<int32_t>(_nextTransition - millis());
    return remaining > 0 ? static_cast<uint16_t>(remaining) : 1;
}

inline void PulseChannel::stopWriter() {
    if (_phase == Phase::Pulse) IO::digitalWritePulse(_index, LOW);
    _phase = Phase::Idle;
    _remaining = 0;
}

} // namespace VMCBuddy
#endif // PULSE_CHANNEL_H