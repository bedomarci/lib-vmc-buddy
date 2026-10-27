#ifndef PULSE_IDC_H
#define PULSE_IDC_H

#include <PulseWriter.h>
#include <PulseReader.h>

namespace VMCBuddy {

/** Adapter for one three-signal pulse IDC connector.
 * Local signals are CH1, CH2, INHIBIT. IDC 1 maps to board channels
 * 1/2/3; IDC 2 maps to board channels 4/5/6.
 *
 * Slave mode reads the optoisolated IDC inputs. Master mode writes
 * through the matching PULSE-OUT-TTL shift-register paths. The current
 * schematic does not route those output nets back to the IDC pins. */
class PulseIDC : public PulseWriter {
public:
    enum class Mode : uint8_t { Master, Slave };

    explicit PulseIDC(uint8_t connectorNumber, Mode mode = Mode::Slave);

    bool begin() override;
    void end() override;
    const char* name() const override;

    /** Change direction mode before begin(); false if already active. */
    bool setMode(Mode mode);
    Mode mode() const { return _mode; }

    /** 1-based local channel maps to a global zero-based pulse index. */
    int pulseIndex(uint8_t localChannel) const;
    /** IDC contact number: local CH1, CH2, INHIBIT are pins 8, 6, 4. */
    int idcPin(uint8_t localChannel) const;
    int gpio(uint8_t localChannel) const;

    /** Read channel or inhibit in Slave mode; returns -1 otherwise. */
    int read(uint8_t localChannel) const;
    int readInhibit() const { return read(3); }

    /** Configure the matching PulseReader input in Slave mode. */
    bool setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                          uint16_t debounceMs = DEFAULT_PULSE_DEBOUNCE);

    /** Drive inhibit output in Master mode. */
    bool setInhibit(bool active);

protected:
    bool validTarget(uint8_t localChannel) const override;
    void writeOut(uint8_t localChannel, uint8_t state) override;

private:
    uint8_t _connectorNumber;
    Mode _mode;
};

inline PulseIDC::PulseIDC(uint8_t connectorNumber, Mode mode)
    : PulseWriter(), _connectorNumber(connectorNumber), _mode(mode) {}

inline const char* PulseIDC::name() const {
    return _connectorNumber == 1 ? "PulseIDC1" :
           _connectorNumber == 2 ? "PulseIDC2" : "PulseIDC-invalid";
}

inline bool PulseIDC::setMode(Mode mode) {
    if (_active || (_connectorNumber != 1 && _connectorNumber != 2) ||
        (mode != Mode::Master && mode != Mode::Slave)) return false;
    _mode = mode;
    return true;
}

inline int PulseIDC::pulseIndex(uint8_t localChannel) const {
    if ((_connectorNumber != 1 && _connectorNumber != 2) ||
        localChannel < 1 || localChannel > 3) return -1;
    return (_connectorNumber - 1) * 3 + localChannel - 1;
}

inline int PulseIDC::idcPin(uint8_t localChannel) const {
    if (pulseIndex(localChannel) < 0) return -1;
    return 10 - 2 * localChannel; // local channels 1/2/3 -> IDC pins 8/6/4
}

inline int PulseIDC::gpio(uint8_t localChannel) const {
    const int index = pulseIndex(localChannel);
    return index >= 0 ? boardCfg().pulseInGpio[index] : -1;
}

inline bool PulseIDC::begin() {
    if ((_connectorNumber != 1 && _connectorNumber != 2) ||
        (_mode != Mode::Master && _mode != Mode::Slave)) return false;
    if (!PulseWriter::begin()) return false;
    const uint8_t pinMode = _mode == Mode::Master ? OUTPUT : INPUT;
    for (uint8_t channel = 1; channel <= 3; ++channel)
        IO::pinModePulse(pulseIndex(channel), pinMode);
    return true;
}

inline void PulseIDC::end() {
    if (!_active) return;
    PulseWriter::end();
    for (uint8_t channel = 1; channel <= 3; ++channel) {
        const int index = pulseIndex(channel);
        detachInterrupt(boardCfg().pulseInGpio[index]);
        PulseReader::setInterruptMode(index, DETACH_ISR);
        IO::pinModePulse(index, INPUT);
    }
}

inline int PulseIDC::read(uint8_t localChannel) const {
    const int index = pulseIndex(localChannel);
    if (!_active || _mode != Mode::Slave || index < 0) return -1;
    return IO::digitalReadPulse(index);
}

inline bool PulseIDC::setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                                       uint16_t debounceMs) {
    const int index = pulseIndex(localChannel);
    if (_active || _mode != Mode::Slave || index < 0) return false;
    IO::pinModePulse(index, INPUT);
    PulseReader::setInterruptMode(index, interruptMode, debounceMs);
    return true;
}

inline bool PulseIDC::validTarget(uint8_t localChannel) const {
    return _active && _mode == Mode::Master && pulseIndex(localChannel) >= 0 &&
           Context::getInstance()->pulseConfiguration[pulseIndex(localChannel)].pinMode == OUTPUT;
}

inline void PulseIDC::writeOut(uint8_t localChannel, uint8_t state) {
    const int index = pulseIndex(localChannel);
    if (index >= 0) IO::digitalWritePulse(index, state);
}

inline bool PulseIDC::setInhibit(bool active) {
    if (!validTarget(3)) return false;
    IO::digitalWritePulse(pulseIndex(3), active ? HIGH : LOW);
    return true;
}

} // namespace VMCBuddy
#endif // PULSE_IDC_H
