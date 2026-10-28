#ifndef PULSE_WRITER_H
#define PULSE_WRITER_H

#include <IO.h>

namespace VMCBuddy {

/** Task-free counted pulse behavior. Derived peripherals own scheduling. */
class PulseWriter {
protected:
    PulseWriter() = default;
    ~PulseWriter() = default;

    void startWriter() { _ready = true; }
    void stopWriter();
    bool startPulseTrain(uint8_t index, uint16_t count,
                         uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                         uint16_t startDelayMs = 0);
    void updateWriter();
    bool writerBusy() const { return _phase != Phase::Idle; }
    uint16_t writerDelayMs() const;

    virtual bool validTarget(uint8_t index) const;
    virtual void writeOut(uint8_t index, uint8_t state);

private:
    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };
    bool _ready = false;
    uint8_t _target = 0;
    uint16_t _remaining = 0;
    uint16_t _pulseLengthMs = 0;
    uint16_t _pauseLengthMs = 0;
    uint32_t _nextTransition = 0;
    Phase _phase = Phase::Idle;
};

inline void PulseWriter::stopWriter() {
    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    _phase = Phase::Idle;
    _remaining = 0;
    _ready = false;
}

inline bool PulseWriter::validTarget(uint8_t index) const {
    auto* context = Context::getInstance();
    return index < context->boardConfig.numPulses &&
           context->pulseConfiguration[index].pinMode == OUTPUT;
}

inline void PulseWriter::writeOut(uint8_t index, uint8_t state) {
    IO::digitalWritePulse(index, state);
}

inline bool PulseWriter::startPulseTrain(uint8_t index, uint16_t count,
                                         uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                                         uint16_t startDelayMs) {
    if (!_ready || !validTarget(index) || count == 0 || pulseLengthMs == 0)
        return false;

    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    _target = index;
    _remaining = count;
    _pulseLengthMs = pulseLengthMs;
    _pauseLengthMs = pauseLengthMs;
    _phase = Phase::Start;
    _nextTransition = millis() + startDelayMs;
    return true;
}

inline void PulseWriter::updateWriter() {
    if (!_ready || _phase == Phase::Idle ||
        static_cast<int32_t>(millis() - _nextTransition) < 0) return;

    const uint32_t now = millis();
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        writeOut(_target, HIGH);
        _phase = Phase::Pulse;
        _nextTransition = now + _pulseLengthMs;
        break;
    case Phase::Pulse:
        writeOut(_target, LOW);
        if (--_remaining == 0) {
            _phase = Phase::Idle;
        } else {
            _phase = Phase::Pause;
            _nextTransition = now + _pauseLengthMs;
        }
        break;
    case Phase::Idle:
        break;
    }
}

inline uint16_t PulseWriter::writerDelayMs() const {
    if (_phase == Phase::Idle) return 1;
    const int32_t remaining = static_cast<int32_t>(_nextTransition - millis());
    return remaining > 0 ? static_cast<uint16_t>(remaining) : 1;
}

} // namespace VMCBuddy
#endif // PULSE_WRITER_H
