#ifndef PULSE_PERIPHERAL_H
#define PULSE_PERIPHERAL_H

#include <IPeripheral.h>
#include <PulseReader.h>
#include <PulseWriter.h>

namespace VMCBuddy {

/** One schedulable owner for a mapped pulse connector. */
class PulsePeripheral : public IPeripheral, public PulseReader, public PulseWriter {
public:
    enum class Mode : uint8_t { Master, Slave };

    bool begin() override;
    void end() override;

    bool setMode(Mode mode);
    Mode mode() const { return _mode; }
    virtual bool setInhibitChannel(uint8_t localChannel);
    uint8_t inhibitChannel() const { return _inhibitChannel; }

    int pulseIndex(uint8_t localChannel) const;
    int gpio(uint8_t localChannel) const;
    int read(uint8_t localChannel) const;
    int readInhibit() const;
    bool setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                          uint16_t debounceMs = DEFAULT_PULSE_DEBOUNCE);
    bool sendPulses(uint8_t localChannel, uint16_t count, uint16_t pulseLengthMs,
                    uint16_t pauseLengthMs, uint16_t startDelayMs = 0);
    bool setInhibit(bool active);

protected:
    PulsePeripheral(Mode mode, uint8_t inhibitChannel);
    virtual uint8_t channelCount() const = 0;
    virtual int mapChannel(uint8_t localChannel) const = 0;
    bool isInputChannel(uint8_t localChannel) const;
    bool validTarget(uint8_t localChannel) const override;
    void writeOut(uint8_t localChannel, uint8_t state) override;

private:
    void tick();
    bool _readerPolling = false;
    Mode _mode;
    uint8_t _inhibitChannel;
};

inline PulsePeripheral::PulsePeripheral(Mode mode, uint8_t inhibitChannel)
    : IPeripheral(), PulseReader(), PulseWriter(),
      _mode(mode), _inhibitChannel(inhibitChannel) {
    setInterval(1);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
}

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
    const auto& cfg = Context::getInstance()->boardConfig;
    return index >= 0 && index < cfg.numPulses ? cfg.pulseInGpio[index] : -1;
}

inline bool PulsePeripheral::begin() {
    if (_active) return true;
    if ((_mode != Mode::Master && _mode != Mode::Slave) ||
        _inhibitChannel > channelCount()) return false;

    auto* context = Context::getInstance();
    for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
        const int index = mapChannel(channel);
        if (index < 0 || index >= context->boardConfig.numPulses ||
            context->pulseConfiguration[index].owner) return false;
        for (uint8_t prior = 1; prior < channel; ++prior)
            if (mapChannel(prior) == index) return false;
    }

    _readerPolling = false;
    for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
        const uint8_t index = static_cast<uint8_t>(mapChannel(channel));
        auto& pulse = context->pulseConfiguration[index];
        pulse.owner = this;
        pulse.isDirty = false;
        const bool input = isInputChannel(channel);
        IO::pinModePulse(index, input ? INPUT : OUTPUT);
        if (input) {
            if (PulseReader::interruptEnabled(index)) {
                PulseReader::attachInterruptFor(index);
                _readerPolling = true;
            } else {
                PulseReader::detachInterruptFor(index);
            }
        } else {
            PulseReader::detachInterruptFor(index);
            PulseReader::setInterruptMode(index, DETACH_ISR);
            IO::digitalWritePulse(index, LOW);
        }
    }

    PulseWriter::startWriter();
    _active = true;
    if (_readerPolling) Task::enable();
    return true;
}

inline void PulsePeripheral::end() {
    if (!_active) return;
    Task::disable();
    PulseWriter::stopWriter();

    auto* context = Context::getInstance();
    for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
        const int index = mapChannel(channel);
        if (index < 0 || context->pulseConfiguration[index].owner != this) continue;
        PulseReader::detachInterruptFor(static_cast<uint8_t>(index));
        IO::pinModePulse(static_cast<uint8_t>(index), INPUT);
        PulseReader::setInterruptMode(static_cast<uint8_t>(index), DETACH_ISR);
        context->pulseConfiguration[index].isDirty = false;
        context->pulseConfiguration[index].owner = nullptr;
    }
    _readerPolling = false;
    _active = false;
}

inline int PulsePeripheral::read(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    if (!_active || index < 0 || index >= Context::getInstance()->boardConfig.numPulses ||
        !isInputChannel(localChannel) ||
        Context::getInstance()->pulseConfiguration[index].owner != this) return -1;
    return IO::digitalReadPulse(static_cast<uint8_t>(index));
}

inline int PulsePeripheral::readInhibit() const {
    return _inhibitChannel && isInputChannel(_inhibitChannel) ? read(_inhibitChannel) : -1;
}

inline bool PulsePeripheral::setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                                               uint16_t debounceMs) {
    const int index = mapChannel(localChannel);
    const auto& cfg = Context::getInstance()->boardConfig;
    if (_active || index < 0 || index >= cfg.numPulses ||
        Context::getInstance()->pulseConfiguration[index].owner ||
        !isInputChannel(localChannel)) return false;
    IO::pinModePulse(static_cast<uint8_t>(index), INPUT);
    return PulseReader::setInterruptMode(static_cast<uint8_t>(index), interruptMode, debounceMs);
}

inline bool PulsePeripheral::sendPulses(uint8_t localChannel, uint16_t count,
                                        uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                                        uint16_t startDelayMs) {
    if (!_active || !PulseWriter::startPulseTrain(localChannel, count, pulseLengthMs,
                                                  pauseLengthMs, startDelayMs)) return false;
    if (!_readerPolling) {
        Task::disable();
        setInterval(PulseWriter::writerDelayMs());
        Task::enable();
    }
    return true;
}

inline bool PulsePeripheral::validTarget(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    return _active && index >= 0 &&
           index < Context::getInstance()->boardConfig.numPulses &&
           !isInputChannel(localChannel) &&
           Context::getInstance()->pulseConfiguration[index].owner == this &&
           Context::getInstance()->pulseConfiguration[index].pinMode == OUTPUT;
}

inline bool PulsePeripheral::isInputChannel(uint8_t localChannel) const {
    if (mapChannel(localChannel) < 0) return false;
    const bool isInhibit = localChannel == _inhibitChannel;
    return isInhibit ? _mode == Mode::Master : _mode == Mode::Slave;
}

inline void PulsePeripheral::writeOut(uint8_t localChannel, uint8_t state) {
    const int index = mapChannel(localChannel);
    if (index >= 0) IO::digitalWritePulse(static_cast<uint8_t>(index), state);
}

inline bool PulsePeripheral::setInhibit(bool active) {
    if (!_inhibitChannel || !validTarget(_inhibitChannel)) return false;
    IO::digitalWritePulse(static_cast<uint8_t>(mapChannel(_inhibitChannel)), active ? HIGH : LOW);
    return true;
}

inline void PulsePeripheral::tick() {
    if (!_active) return;
    if (_readerPolling) {
        auto* context = Context::getInstance();
        for (uint8_t channel = 1; channel <= channelCount(); ++channel) {
            const int index = mapChannel(channel);
            if (index >= 0 && isInputChannel(channel) &&
                context->pulseConfiguration[index].owner == this)
                PulseReader::checkDirty(static_cast<uint8_t>(index));
        }
    }

    PulseWriter::updateWriter();
    if (!_readerPolling) {
        if (PulseWriter::writerBusy()) setInterval(PulseWriter::writerDelayMs());
        else Task::disable();
    }
}

} // namespace VMCBuddy
#endif // PULSE_PERIPHERAL_H
