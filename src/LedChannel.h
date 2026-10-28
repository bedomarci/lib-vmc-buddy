#ifndef LED_CHANNEL_H
#define LED_CHANNEL_H

#include <IPeripheral.h>
#include <IO.h>
#include <EventHandler.h>

namespace VMCBuddy {

class LedChannel : public IPeripheral {
public:
    LedChannel(const char* name, uint8_t index, uint8_t srBit);

    void boot(Context* ctx) override;
    bool begin() override;
    void end() override;
    const char* name() const override { return _name; }

    bool isOutput() const { return true; }

    bool set(uint8_t state);
    bool blink(uint16_t count, uint16_t onTimeMs, uint16_t offTimeMs,
               uint16_t startDelayMs = 0);

private:
    void tick();

    const char* _name;
    uint8_t _index;
    uint8_t _srBit;
    bool _state = false;

    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };
    Phase _phase = Phase::Idle;
    uint16_t _remaining = 0;
    uint16_t _onMs = 0;
    uint16_t _offMs = 0;
    uint32_t _nextTransition = 0;
};

inline LedChannel::LedChannel(const char* name, uint8_t index, uint8_t srBit)
    : _name(name), _index(index), _srBit(srBit) {
    setInterval(1);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
    Task::disable();
}

inline void LedChannel::boot(Context* ctx) {
    ctx->board()->addPeripheral(this);
}

inline bool LedChannel::begin() {
    if (_active) return true;
    IO::setShiftRegisterBit(_srBit, LOW);
    _state = false;
    _active = true;
    return true;
}

inline void LedChannel::end() {
    Task::disable();
    if (_phase == Phase::Pulse) IO::setShiftRegisterBit(_srBit, LOW);
    _phase = Phase::Idle;
    _remaining = 0;
    _active = false;
}

inline bool LedChannel::set(uint8_t state) {
    if (!_active) return false;
    IO::setShiftRegisterBit(_srBit, state ? LOW : HIGH);
    _state = state;
    return true;
}

inline bool LedChannel::blink(uint16_t count, uint16_t onTimeMs,
                              uint16_t offTimeMs, uint16_t startDelayMs) {
    if (!_active || count == 0 || onTimeMs == 0) return false;

    if (_phase == Phase::Pulse) IO::setShiftRegisterBit(_srBit, LOW);
    _remaining = count;
    _onMs = onTimeMs;
    _offMs = offTimeMs;
    _phase = Phase::Start;
    _nextTransition = millis() + startDelayMs;

    EventHandler::trigger(Event::LED_START, _index, count);
    Task::disable();
    setInterval(startDelayMs + 1);
    Task::enable();
    return true;
}

inline void LedChannel::tick() {
    if (_phase == Phase::Idle ||
        static_cast<int32_t>(millis() - _nextTransition) < 0) return;

    const uint32_t now = millis();
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        IO::setShiftRegisterBit(_srBit, LOW); // active-low: LOW = on
        _phase = Phase::Pulse;
        _nextTransition = now + _onMs;
        EventHandler::trigger(Event::LED_TICK, _index, HIGH);
        setInterval(_onMs);
        break;
    case Phase::Pulse:
        IO::setShiftRegisterBit(_srBit, HIGH); // active-low: HIGH = off
        if (--_remaining == 0) {
            _phase = Phase::Idle;
            EventHandler::trigger(Event::LED_TICK, _index, LOW);
            EventHandler::trigger(Event::LED_STOP, _index);
            Task::disable();
        } else {
            _phase = Phase::Pause;
            _nextTransition = now + _offMs;
            EventHandler::trigger(Event::LED_TICK, _index, LOW);
            setInterval(_offMs);
        }
        break;
    case Phase::Idle:
        break;
    }
}

} // namespace VMCBuddy
#endif // LED_CHANNEL_H