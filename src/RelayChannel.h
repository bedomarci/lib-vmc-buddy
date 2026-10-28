#ifndef RELAY_CHANNEL_H
#define RELAY_CHANNEL_H

#include <PulseChannel.h>

namespace VMCBuddy {

class RelayChannel : public PulseChannel {
public:
    RelayChannel(const char* name, uint8_t boardIndex, uint8_t srBit);

    bool begin() override;
    const char* name() const override { return _name; }
};

inline RelayChannel::RelayChannel(const char* name, uint8_t boardIndex, uint8_t srBit)
    : PulseChannel(name, boardIndex) {
    _srBit = srBit;
}

inline bool RelayChannel::begin() {
    if (_active) return true;
    const auto& cfg = Context::getInstance()->boardConfig;
    _gpio = cfg.pulseInGpio[_boardIndex];
    _pinMode = OUTPUT;
    _configured = true;
    auto* ctx = Context::getInstance();
    if (_boardIndex < ctx->boardConfig.numPulses)
        ctx->pulseChannels[_boardIndex] = this;

    ::pinMode(_gpio, OUTPUT);
    detachInterrupt(_gpio);
    IO::setShiftRegisterBit(_srBit, LOW);
    _value = LOW;
    _active = true;
    return true;
}

} // namespace VMCBuddy
#endif // RELAY_CHANNEL_H