#ifndef PULSE_CHANNEL_H
#define PULSE_CHANNEL_H

#include <IPeripheral.h>
#include <IO.h>
#include <EventHandler.h>

namespace VMCBuddy {

class PulseChannel : public IPeripheral {
public:
    PulseChannel(const char* name, uint8_t boardIndex);

    void configure(uint8_t gpio, uint8_t srBit, uint8_t pinMode,
                   uint8_t interruptMode, uint16_t debounce);

    void boot(Context* ctx) override;
    bool begin() override;
    void end() override;
    const char* name() const override { return _name; }

    bool isOutput() const { return _pinMode == OUTPUT; }
    uint8_t pinMode() const { return _pinMode; }
    uint8_t boardIndex() const { return _boardIndex; }

    int read() const;
    void write(uint8_t val);
    bool startPulseTrain(uint16_t count, uint16_t pulseLengthMs,
                         uint16_t pauseLengthMs, uint16_t startDelayMs = 0);

protected:
    void tick();

    const char* _name;
    uint8_t _boardIndex;

    uint8_t _gpio = 0;
    uint8_t _srBit = 0;
    uint8_t _interruptMode = DETACH_ISR;
    uint8_t _pinMode = INPUT;
    uint8_t _value = LOW;
    bool _isDirty = false;
    unsigned long _lastInterrupt = 0;
    uint16_t _debounce = DEFAULT_PULSE_DEBOUNCE;

    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };
    Phase _phase = Phase::Idle;
    uint16_t _remaining = 0;
    uint16_t _pulseLengthMs = 0;
    uint16_t _pauseLengthMs = 0;
    uint32_t _nextTransition = 0;

    bool _configured = false;

    static void IRAM_ATTR handleInterruptArg(void* arg);
    static void IRAM_ATTR handleInterrupts(uint8_t pulseIndex);
};

inline PulseChannel::PulseChannel(const char* name, uint8_t boardIndex)
    : _name(name), _boardIndex(boardIndex) {
    setInterval(1);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
    Task::disable();
}

inline void PulseChannel::configure(uint8_t gpio, uint8_t srBit, uint8_t pinMode,
                                     uint8_t interruptMode, uint16_t debounce) {
    if (_active || _configured) return;
    _gpio = gpio;
    _srBit = srBit;
    _pinMode = pinMode;
    _interruptMode = interruptMode;
    _debounce = debounce;
    _configured = true;
    auto* ctx = Context::getInstance();
    if (_boardIndex < ctx->boardConfig.numPulses)
        ctx->pulseChannels[_boardIndex] = this;
}

inline void PulseChannel::boot(Context* ctx) {
    ctx->board()->addPeripheral(this);
}

inline bool PulseChannel::begin() {
    if (_active) return true;
    if (!_configured) return false;

    ::pinMode(_gpio, _pinMode);
    if (_pinMode == INPUT) {
        _isDirty = false;
        if (_interruptMode != DETACH_ISR) {
            attachInterruptArg(_gpio, handleInterruptArg,
                               reinterpret_cast<void*>(static_cast<uintptr_t>(_boardIndex + 1)),
                               _interruptMode);
            Task::enable();
        }
    } else {
        detachInterrupt(_gpio);
        IO::setShiftRegisterBit(_srBit, LOW);
        _value = LOW;
    }
    _active = true;
    return true;
}

inline void PulseChannel::end() {
    if (!_active) return;
    Task::disable();
    if (_phase == Phase::Pulse) write(LOW);
    if (_pinMode == INPUT) detachInterrupt(_gpio);
    _phase = Phase::Idle;
    _remaining = 0;
    _isDirty = false;
    auto* ctx = Context::getInstance();
    if (_boardIndex < ctx->boardConfig.numPulses)
        ctx->pulseChannels[_boardIndex] = nullptr;
    _active = false;
    _configured = false;
}

inline int PulseChannel::read() const {
    if (!_active || _pinMode != INPUT) return -1;
    return digitalRead(_gpio);
}

inline void PulseChannel::write(uint8_t val) {
    if (!_active || _pinMode != OUTPUT) return;
    IO::setShiftRegisterBit(_srBit, val ? HIGH : LOW);
    _value = val;
}

inline bool PulseChannel::startPulseTrain(uint16_t count, uint16_t pulseLengthMs,
                                          uint16_t pauseLengthMs, uint16_t startDelayMs) {
    if (!_active || _pinMode != OUTPUT || count == 0 || pulseLengthMs == 0)
        return false;
    if (_phase == Phase::Pulse) write(LOW);
    _remaining = count;
    _pulseLengthMs = pulseLengthMs;
    _pauseLengthMs = pauseLengthMs;
    _phase = Phase::Start;
    _nextTransition = millis() + startDelayMs;
    EventHandler::trigger(Event::CHANNEL_START, _boardIndex, count);
    Task::enable();
    return true;
}

inline void PulseChannel::tick() {
    if (!_active) return;

    if (_pinMode == INPUT) {
        if (_isDirty) {
            EventHandler::trigger(Event::PULSE_INT, _boardIndex, _value, _interruptMode);
            _isDirty = false;
        }
        return;
    }

    if (_phase == Phase::Idle ||
        static_cast<int32_t>(millis() - _nextTransition) < 0) return;

    const uint32_t now = millis();
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        write(HIGH);
        _phase = Phase::Pulse;
        _nextTransition = now + _pulseLengthMs;
        EventHandler::trigger(Event::CHANNEL_TICK, _boardIndex, HIGH);
        setInterval(_pulseLengthMs);
        break;
    case Phase::Pulse:
        write(LOW);
        if (--_remaining == 0) {
            _phase = Phase::Idle;
            EventHandler::trigger(Event::CHANNEL_TICK, _boardIndex, LOW);
            EventHandler::trigger(Event::CHANNEL_STOP, _boardIndex);
            Task::disable();
        } else {
            _phase = Phase::Pause;
            _nextTransition = now + _pauseLengthMs;
            EventHandler::trigger(Event::CHANNEL_TICK, _boardIndex, LOW);
            setInterval(_pauseLengthMs);
        }
        break;
    case Phase::Idle:
        break;
    }
}

inline void IRAM_ATTR PulseChannel::handleInterruptArg(void* arg) {
    const uintptr_t idx = reinterpret_cast<uintptr_t>(arg);
    if (idx == 0 || idx > Context::getInstance()->boardConfig.numPulses) return;
    handleInterrupts(static_cast<uint8_t>(idx - 1));
}

inline void IRAM_ATTR PulseChannel::handleInterrupts(const uint8_t pulseIndex) {
    auto* ctx = Context::getInstance();
    if (pulseIndex >= ctx->boardConfig.numPulses) return;
    auto* ch = ctx->pulseChannels[pulseIndex];
    if (!ch || ch->_pinMode != INPUT) return;
    const unsigned long now = millis();
    if (now - ch->_lastInterrupt > ch->_debounce) {
        ch->_value = digitalRead(ctx->boardConfig.pulseInGpio[pulseIndex]);
        ch->_isDirty = true;
        ch->_lastInterrupt = now;
    }
}

} // namespace VMCBuddy
#endif // PULSE_CHANNEL_H