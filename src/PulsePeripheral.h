#ifndef PULSE_PERIPHERAL_H
#define PULSE_PERIPHERAL_H

#include <PulseWriter.h>
#include <PulseReader.h>

namespace VMCBuddy {

/** Shared direction and inhibit behavior for pulse connectors.
 * Slave mode reads optoisolated inputs; Master mode writes the
 * corresponding PULSE-OUT-TTL shift-register channels. */
class PulsePeripheral : public PulseWriter {
public:
    enum class Mode : uint8_t { Master, Slave };

    bool begin() override;
    void end() override;

    /** Select mode or inhibit channel before begin(). Channel numbering
     * is connector-local and 1-based; zero disables inhibit assignment. */
    bool setMode(Mode mode);
    Mode mode() const { return _mode; }
    bool setInhibitChannel(uint8_t localChannel);
    uint8_t inhibitChannel() const { return _inhibitChannel; }

    int pulseIndex(uint8_t localChannel) const;
    int gpio(uint8_t localChannel) const;
    int read(uint8_t localChannel) const;
    int readInhibit() const;
    bool setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                          uint16_t debounceMs = DEFAULT_PULSE_DEBOUNCE);
    bool setInhibit(bool active);

protected:
    PulsePeripheral(Mode mode, uint8_t inhibitChannel);
    virtual uint8_t channelCount() const = 0;
    virtual int mapChannel(uint8_t localChannel) const = 0;
    bool validTarget(uint8_t localChannel) const override;
    void writeOut(uint8_t localChannel, uint8_t state) override;

private:
    Mode _mode;
    uint8_t _inhibitChannel;
};

inline PulsePeripheral::PulsePeripheral(Mode mode, uint8_t inhibitChannel)
    : PulseWriter(), _mode(mode), _inhibitChannel(inhibitChannel) {}

inline bool PulsePeripheral::setMode(Mode mode) {
    if (_active || (mode != Mode::Master && mode != Mode::Slave)) return false;
    _mode = mode;
    return true;
}

inline bool PulsePeripheral::setInhibitChannel(uint8_t localChannel) {
    if (_active || localChannel > channelCount()) return false;
    _inhibitChannel = localChannel;
    return true;
}

inline int PulsePeripheral::pulseIndex(uint8_t localChannel) const {
    return mapChannel(localChannel);
}

inline int PulsePeripheral::gpio(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    return index >= 0 ? boardCfg().pulseInGpio[index] : -1;
}

inline bool PulsePeripheral::begin() {
    if ((_mode != Mode::Master && _mode != Mode::Slave) ||
        _inhibitChannel > channelCount() || !PulseWriter::begin()) return false;

    const uint8_t pinMode = _mode == Mode::Master ? OUTPUT : INPUT;
    for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
        const int index = mapChannel(channel);
        if (index < 0) { PulseWriter::end(); return false; }
        IO::pinModePulse(index, pinMode);
    }
    return true;
}

inline void PulsePeripheral::end() {
    if (!_active) return;
    PulseWriter::end();
    for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
        const int index = mapChannel(channel);
        if (index < 0) continue;
        detachInterrupt(boardCfg().pulseInGpio[index]);
        PulseReader::setInterruptMode(index, DETACH_ISR);
        IO::pinModePulse(index, INPUT);
    }
}

inline int PulsePeripheral::read(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    if (!_active || _mode != Mode::Slave || index < 0) return -1;
    return IO::digitalReadPulse(index);
}

inline int PulsePeripheral::readInhibit() const {
    return _inhibitChannel ? read(_inhibitChannel) : -1;
}

inline bool PulsePeripheral::setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                                               uint16_t debounceMs) {
    const int index = mapChannel(localChannel);
    if (_active || _mode != Mode::Slave || index < 0) return false;
    IO::pinModePulse(index, INPUT);
    PulseReader::setInterruptMode(index, interruptMode, debounceMs);
    return true;
}

inline bool PulsePeripheral::validTarget(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    return _active && _mode == Mode::Master && index >= 0 &&
           Context::getInstance()->pulseConfiguration[index].pinMode == OUTPUT;
}

inline void PulsePeripheral::writeOut(uint8_t localChannel, uint8_t state) {
    const int index = mapChannel(localChannel);
    if (index >= 0) IO::digitalWritePulse(index, state);
}

inline bool PulsePeripheral::setInhibit(bool active) {
    if (!_inhibitChannel || !validTarget(_inhibitChannel)) return false;
    IO::digitalWritePulse(mapChannel(_inhibitChannel), active ? HIGH : LOW);
    return true;
}

} // namespace VMCBuddy
#endif // PULSE_PERIPHERAL_H
