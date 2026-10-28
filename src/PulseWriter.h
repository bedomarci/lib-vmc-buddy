#ifndef PULSE_WRITER_H
#define PULSE_WRITER_H

#include <IPeripheral.h>
#include <IO.h>

namespace VMCBuddy {

/** Reusable scheduled pulse generator for the eight pulse outputs.
 * Register one instance with VMCBuddy; sendPulses() restarts its task
 * for a finite pulse count. Durations are milliseconds. */
class PulseWriter : public IPeripheral {
public:
    PulseWriter();

    bool begin() override;
    void end() override;
    const char* name() const override { return "PulseWriter"; }

    /** Start/restart a counted pulse train. Returns false for invalid
     * channel, non-output mode, zero count, or zero pulse length. */
    bool sendPulses(uint8_t pulseIndex, uint16_t count,
                    uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                    uint16_t startDelayMs = 0);

protected:
    virtual bool validTarget(uint8_t index) const;
    virtual void writeOut(uint8_t index, uint8_t state);

private:
    enum class Phase : uint8_t { Idle, Start, Pulse, Pause };
    void tick();

    uint8_t _target = 0;
    uint16_t _remaining = 0;
    uint16_t _pulseLengthMs = 0;
    uint16_t _pauseLengthMs = 0;
    Phase _phase = Phase::Idle;
};

inline PulseWriter::PulseWriter() : IPeripheral() {
    setInterval(TASK_IMMEDIATE);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
}

inline bool PulseWriter::begin() {
    _active = true;
    return true;
}

inline void PulseWriter::end() {
    Task::disable();
    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    _phase = Phase::Idle;
    _remaining = 0;
    _active = false;
}

inline bool PulseWriter::validTarget(uint8_t index) const {
    return index < Context::getInstance()->boardConfig.numPulses &&
           Context::getInstance()->pulseConfiguration[index].pinMode == OUTPUT;
}

inline void PulseWriter::writeOut(uint8_t index, uint8_t state) {
    IO::digitalWritePulse(index, state);
}

inline bool PulseWriter::sendPulses(uint8_t pulseIndex, uint16_t count,
                                    uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                                    uint16_t startDelayMs) {
    if (!_active || !validTarget(pulseIndex) || count == 0 || pulseLengthMs == 0)
        return false;

    if (_phase == Phase::Pulse) writeOut(_target, LOW);
    Task::disable();

    _target = pulseIndex;
    _remaining = count;
    _pulseLengthMs = pulseLengthMs;
    _pauseLengthMs = pauseLengthMs;
    _phase = Phase::Start;
    setInterval(startDelayMs ? startDelayMs : TASK_IMMEDIATE);
    Task::enable();
    return true;
}

inline void PulseWriter::tick() {
    switch (_phase) {
    case Phase::Start:
    case Phase::Pause:
        writeOut(_target, HIGH);
        _phase = Phase::Pulse;
        setInterval(_pulseLengthMs);
        break;
    case Phase::Pulse:
        writeOut(_target, LOW);
        if (--_remaining == 0) {
            _phase = Phase::Idle;
            Task::disable();
        } else {
            _phase = Phase::Pause;
            setInterval(_pauseLengthMs);
        }
        break;
    case Phase::Idle:
        Task::disable();
        break;
    }
}

} // namespace VMCBuddy
#endif // PULSE_WRITER_H
