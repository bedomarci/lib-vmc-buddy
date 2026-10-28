#ifndef PULSE_PERIPHERAL_H
#define PULSE_PERIPHERAL_H

#include <IPeripheral.h>
#include <PulseChannel.h>

namespace VMCBuddy {

class PulsePeripheral : public IPeripheral {
public:
    enum class Mode : uint8_t { Master, Slave };

    bool begin() override;
    void end() override;

    bool setMode(Mode mode);
    Mode mode() const { return _mode; }
    virtual bool setInhibitChannel(uint8_t localChannel);
    uint8_t inhibitChannel() const { return _inhibitChannel; }

    int gpio(uint8_t localChannel) const;
    int pulseIndex(uint8_t localChannel) const { return mapChannel(localChannel); }
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
    PulseChannel* channel(uint8_t localChannel);

private:
    void tick();
    bool _readerPolling = false;
    Mode _mode;
    uint8_t _inhibitChannel;
    PulseChannel* _channels[8] = {}; // ponytail: max 8 pulse channels on board
};

inline PulsePeripheral::PulsePeripheral(Mode mode, uint8_t inhibitChannel)
    : _mode(mode), _inhibitChannel(inhibitChannel) {
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

inline int PulsePeripheral::gpio(uint8_t localChannel) const {
    const int index = mapChannel(localChannel);
    const auto& cfg = Context::getInstance()->boardConfig;
    return index >= 0 && index < cfg.numPulses ? cfg.pulseInGpio[index] : -1;
}

inline PulseChannel* PulsePeripheral::channel(uint8_t localChannel) {
    const int idx = mapChannel(localChannel);
    if (idx < 0) return nullptr;
    return _channels[static_cast<uint8_t>(idx)];
}

inline bool PulsePeripheral::begin() {
    if (_active) return true;
    if ((_mode != Mode::Master && _mode != Mode::Slave) ||
        _inhibitChannel > channelCount()) return false;

    auto* ctx = Context::getInstance();
    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const int idx = mapChannel(c);
        if (idx < 0 || idx >= ctx->boardConfig.numPulses ||
            ctx->pulseConfiguration[idx].owner) return false;
        for (uint8_t prior = 1; prior < c; ++prior)
            if (mapChannel(prior) == idx) return false;
    }

    _readerPolling = false;
    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const uint8_t idx = static_cast<uint8_t>(mapChannel(c));
        _channels[idx] = new PulseChannel(idx, ctx); // ponytail: heap, ~small footprint
        auto& ch = *_channels[idx];
        auto& pulse = ctx->pulseConfiguration[idx];
        ch.setOwner(this);
        pulse.isDirty = false;
        const bool input = isInputChannel(c);
        ch.setPinMode(input ? INPUT : OUTPUT);
        if (input) {
            if (pulse.interruptMode != DETACH_ISR) {
                ch.attachInterrupt();
                _readerPolling = true;
            } else {
                ch.disableInterrupt();
            }
        } else {
            ch.disableInterrupt();
            pulse.interruptMode = DETACH_ISR;
            ch.write(LOW);
        }
    }

    _active = true;
    if (_readerPolling) Task::enable();
    return true;
}

inline void PulsePeripheral::end() {
    if (!_active) return;
    Task::disable();

    auto* ctx = Context::getInstance();
    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const uint8_t idx = static_cast<uint8_t>(mapChannel(c));
        auto* ch = _channels[idx];
        if (ch && ctx->pulseConfiguration[idx].owner == this) {
            ch->disableInterrupt();
            ch->setPinMode(INPUT);
            ch->setInterruptMode(DETACH_ISR);
            ch->stopWriter();
            ctx->pulseConfiguration[idx].isDirty = false;
            ctx->pulseConfiguration[idx].owner = nullptr;
            delete ch;
            _channels[idx] = nullptr;
        }
    }
    _readerPolling = false;
    _active = false;
}

inline int PulsePeripheral::read(uint8_t localChannel) const {
    const int idx = mapChannel(localChannel);
    if (!_active || idx < 0 || idx >= Context::getInstance()->boardConfig.numPulses ||
        !isInputChannel(localChannel) ||
        Context::getInstance()->pulseConfiguration[idx].owner != this) return -1;
    return IO::digitalReadPulse(static_cast<uint8_t>(idx));
}

inline int PulsePeripheral::readInhibit() const {
    return _inhibitChannel && isInputChannel(_inhibitChannel) ? read(_inhibitChannel) : -1;
}

inline bool PulsePeripheral::setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                                               uint16_t debounceMs) {
    const int idx = mapChannel(localChannel);
    const auto& cfg = Context::getInstance()->boardConfig;
    if (_active || idx < 0 || idx >= cfg.numPulses ||
        Context::getInstance()->pulseConfiguration[idx].owner ||
        !isInputChannel(localChannel)) return false;
    auto* ch = _channels[static_cast<uint8_t>(idx)];
    if (!ch) return false;
    ch->setPinMode(INPUT);
    return ch->setInterruptMode(interruptMode, debounceMs);
}

inline bool PulsePeripheral::sendPulses(uint8_t localChannel, uint16_t count,
                                        uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                                        uint16_t startDelayMs) {
    const int idx = mapChannel(localChannel);
    if (!_active || idx < 0) return false;
    auto* ch = _channels[static_cast<uint8_t>(idx)];
    if (!ch || !ch->startPulseTrain(count, pulseLengthMs, pauseLengthMs, startDelayMs)) return false;
    if (!_readerPolling) {
        Task::disable();
        setInterval(ch->writerDelayMs());
        Task::enable();
    }
    return true;
}

inline bool PulsePeripheral::isInputChannel(uint8_t localChannel) const {
    if (mapChannel(localChannel) < 0) return false;
    const bool isInhibit = localChannel == _inhibitChannel;
    return isInhibit ? _mode == Mode::Master : _mode == Mode::Slave;
}

inline bool PulsePeripheral::setInhibit(bool active) {
    if (!_inhibitChannel) return false;
    auto* ch = channel(_inhibitChannel);
    if (!ch || ch->pinMode() != OUTPUT) return false;
    ch->write(active ? HIGH : LOW);
    return true;
}

inline void PulsePeripheral::tick() {
    if (!_active) return;
    if (_readerPolling) {
        for (uint8_t c = 1; c <= channelCount(); ++c) {
            const int idx = mapChannel(c);
            if (idx >= 0 && isInputChannel(c)) {
                auto* ch = _channels[static_cast<uint8_t>(idx)];
                if (ch) ch->checkDirty();
            }
        }
    }

    bool anyBusy = false;
    uint16_t minDelay = 1;
    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const int idx = mapChannel(c);
        if (idx < 0) continue;
        auto* ch = _channels[static_cast<uint8_t>(idx)];
        if (!ch || ch->isInput()) continue;
        ch->updateWriter();
        if (ch->writerBusy()) {
            anyBusy = true;
            const auto d = ch->writerDelayMs();
            if (d < minDelay) minDelay = d;
        }
    }
    if (!_readerPolling) {
        if (anyBusy) setInterval(minDelay);
        else Task::disable();
    }
}

} // namespace VMCBuddy
#endif // PULSE_PERIPHERAL_H