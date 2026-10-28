#ifndef PULSE_PERIPHERAL_H
#define PULSE_PERIPHERAL_H

#include <IPeripheral.h>
#include <PulseChannel.h>

namespace VMCBuddy {

class PulsePeripheral : public IPeripheral {
public:
    enum class Mode : uint8_t { Master, Slave };

    void boot(Context* ctx) override;
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

protected:
    Mode _mode;
    uint8_t _inhibitChannel;
    PulseChannel* _channels[8] = {};
};

inline PulsePeripheral::PulsePeripheral(Mode mode, uint8_t inhibitChannel)
    : _mode(mode), _inhibitChannel(inhibitChannel) {
    Task::disable();
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
    if (idx < 0 || idx >= Context::getInstance()->boardConfig.numPulses) return nullptr;
    return _channels[static_cast<uint8_t>(idx)];
}

inline void PulsePeripheral::boot(Context* ctx) {
    auto* c = Context::getInstance();
    for (uint8_t ch = 1; ch <= channelCount(); ++ch) {
        const int idx = mapChannel(ch);
        if (idx < 0 || idx >= c->boardConfig.numPulses) continue;
        auto* pulse = _channels[static_cast<uint8_t>(idx)];
        if (!pulse) {
            char nameBuf[16];
            snprintf(nameBuf, sizeof(nameBuf), "%s_CH%u", name(), ch);
            pulse = new PulseChannel(nameBuf, static_cast<uint8_t>(idx));
            _channels[static_cast<uint8_t>(idx)] = pulse;
        }
        pulse->boot(ctx);
    }
}

inline bool PulsePeripheral::begin() {
    if (_active) return true;
    if ((_mode != Mode::Master && _mode != Mode::Slave) ||
        _inhibitChannel > channelCount()) return false;

    const auto& cfg = Context::getInstance()->boardConfig;
    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const int idx = mapChannel(c);
        if (idx < 0 || idx >= cfg.numPulses) return false;
        auto* pulse = _channels[static_cast<uint8_t>(idx)];
        if (!pulse) return false;
    }

    for (uint8_t c = 1; c <= channelCount(); ++c) {
        const int idx = mapChannel(c);
        auto* pulse = _channels[static_cast<uint8_t>(idx)];
        const bool isInput = isInputChannel(c);
        const uint8_t pinMode = isInput ? INPUT : OUTPUT;
        const uint8_t interruptMode = isInput ? DETACH_ISR : DETACH_ISR;
        pulse->configure(cfg.pulseInGpio[static_cast<uint8_t>(idx)],
                         cfg.pulseOutSrBit[static_cast<uint8_t>(idx)],
                         pinMode, interruptMode, cfg.defaultPulseDebounce);
    }

    _active = true;
    return true;
}

inline void PulsePeripheral::end() {
    if (!_active) return;
    for (auto* ch : _channels) {
        if (ch) {
            ch->end();
            delete ch;
        }
    }
    memset(_channels, 0, sizeof(_channels));
    _active = false;
}

inline int PulsePeripheral::read(uint8_t localChannel) const {
    const int idx = mapChannel(localChannel);
    if (!_active || idx < 0 || idx >= Context::getInstance()->boardConfig.numPulses ||
        !isInputChannel(localChannel)) return -1;
    auto* ch = _channels[static_cast<uint8_t>(idx)];
    return ch ? ch->read() : -1;
}

inline int PulsePeripheral::readInhibit() const {
    return _inhibitChannel && isInputChannel(_inhibitChannel) ? read(_inhibitChannel) : -1;
}

inline bool PulsePeripheral::setInterruptMode(uint8_t localChannel, uint8_t interruptMode,
                                               uint16_t debounceMs) {
    const int idx = mapChannel(localChannel);
    const auto& cfg = Context::getInstance()->boardConfig;
    if (_active || idx < 0 || idx >= cfg.numPulses ||
        !isInputChannel(localChannel)) return false;
    auto* ch = _channels[static_cast<uint8_t>(idx)];
    if (!ch) return false;
    ch->configure(cfg.pulseInGpio[static_cast<uint8_t>(idx)],
                  cfg.pulseOutSrBit[static_cast<uint8_t>(idx)],
                  INPUT, interruptMode, debounceMs);
    return true;
}

inline bool PulsePeripheral::sendPulses(uint8_t localChannel, uint16_t count,
                                        uint16_t pulseLengthMs, uint16_t pauseLengthMs,
                                        uint16_t startDelayMs) {
    const int idx = mapChannel(localChannel);
    if (!_active || idx < 0) return false;
    auto* ch = _channels[static_cast<uint8_t>(idx)];
    if (!ch || !ch->isOutput()) return false;
    return ch->startPulseTrain(count, pulseLengthMs, pauseLengthMs, startDelayMs);
}

inline bool PulsePeripheral::isInputChannel(uint8_t localChannel) const {
    if (mapChannel(localChannel) < 0) return false;
    const bool isInhibit = localChannel == _inhibitChannel;
    return isInhibit ? _mode == Mode::Master : _mode == Mode::Slave;
}

inline bool PulsePeripheral::setInhibit(bool active) {
    if (!_inhibitChannel) return false;
    auto* ch = channel(_inhibitChannel);
    if (!ch || !ch->isOutput()) return false;
    ch->write(active ? HIGH : LOW);
    return true;
}

} // namespace VMCBuddy
#endif // PULSE_PERIPHERAL_H