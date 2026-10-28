#ifndef LED_H
#define LED_H

#include <IPeripheral.h>
#include <IO.h>
#include <Context.h>

namespace VMCBuddy {

class Led : public IPeripheral {
public:
    Led();
    bool begin() override;
    void end() override;
    const char* name() const override { return "OnboardLEDs"; }

    bool set(uint8_t ledIndex, uint8_t state);
    bool blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
               uint16_t offTimeMs, uint16_t startDelayMs = 0);

private:
    void tick();

    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };
    Phase _phase = Phase::Idle;
    uint8_t _target = 0;
    uint16_t _remaining = 0;
    uint16_t _onMs = 0;
    uint16_t _offMs = 0;
    uint32_t _nextTransition = 0;

    bool validTarget(uint8_t index) const {
        return index < Context::getInstance()->boardConfig.numLeds;
    }
    void writeOut(uint8_t index, uint8_t state) {
        const uint8_t bit = Context::getInstance()->boardConfig.ledSrBit[index];
        IO::setShiftRegisterBit(bit, state ? LOW : HIGH);
    }
};

inline Led::Led() : IPeripheral() {
    setInterval(TASK_IMMEDIATE);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
}

inline bool Led::begin() {
    if (_active) return true;
    _active = true;
    const uint8_t ledCount = Context::getInstance()->boardConfig.numLeds;
    for (uint8_t i = 0; i < ledCount; ++i) set(i, LOW);
    return true;
}

inline void Led::end() {
    Task::disable();
    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    _phase = Phase::Idle;
    _remaining = 0;
    _active = false;
}

inline bool Led::set(uint8_t ledIndex, uint8_t state) {
    if (!validTarget(ledIndex)) return false;
    writeOut(ledIndex, state);
    return true;
}

inline bool Led::blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
                       uint16_t offTimeMs, uint16_t startDelayMs) {
    if (!_active || !validTarget(ledIndex) || count == 0 || onTimeMs == 0)
        return false;

    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    _target = ledIndex;
    _remaining = count;
    _onMs = onTimeMs;
    _offMs = offTimeMs;
    _phase = Phase::Start;
    _nextTransition = millis() + startDelayMs;

    Task::disable();
    setInterval(startDelayMs + 1);
    Task::enable();
    return true;
}

inline void Led::tick() {
    if (_phase == Phase::Idle ||
        static_cast<int32_t>(millis() - _nextTransition) < 0) return;

    const uint32_t now = millis();
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        writeOut(_target, HIGH);
        _phase = Phase::Pulse;
        _nextTransition = now + _onMs;
        setInterval(_onMs);
        break;
    case Phase::Pulse:
        writeOut(_target, LOW);
        if (--_remaining == 0) {
            _phase = Phase::Idle;
            Task::disable();
        } else {
            _phase = Phase::Pause;
            _nextTransition = now + _offMs;
            setInterval(_offMs);
        }
        break;
    case Phase::Idle:
        break;
    }
}

} // namespace VMCBuddy
#endif // LED_H