#ifndef RELAY_H
#define RELAY_H

#include <IPeripheral.h>
#include <IO.h>

namespace VMCBuddy {

/** Controls the two relay-named pulse outputs (channels 7 and 8). */
class Relay : public IPeripheral {
public:
    bool begin() override;
    void end() override;
    const char* name() const override { return "Relay"; }

    /** Relay channel is 1-based: 1 or 2. HIGH activates the output. */
    bool set(uint8_t relayChannel, bool active);

private:
    static uint8_t pulseIndex(uint8_t relayChannel) {
        return static_cast<uint8_t>(NUM_PULSE - 2 + relayChannel - 1);
    }
    static uint8_t shiftRegisterBit(uint8_t relayChannel) {
        return relayChannel == 1 ? PIN_SR_RELAY1 : PIN_SR_RELAY2;
    }
};

inline bool Relay::begin() {
    if (_active) return true;
    const auto& cfg = Context::getInstance()->boardConfig;
    if (cfg.numPulses < 2) return false;

    auto* context = Context::getInstance();
    for (uint8_t channel = 1; channel <= 2; ++channel) {
        const uint8_t index = pulseIndex(channel);
        if (index >= cfg.numPulses || context->pulseConfiguration[index].owner ||
            cfg.pulseOutSrBit[index] != shiftRegisterBit(channel)) return false;
    }

    for (uint8_t channel = 1; channel <= 2; ++channel) {
        const uint8_t index = pulseIndex(channel);
        context->pulseConfiguration[index].owner = this;
        IO::pinModePulse(index, OUTPUT);
    }
    _active = true;
    set(1, false);
    set(2, false);
    return true;
}

inline void Relay::end() {
    if (!_active) return;
    auto* context = Context::getInstance();
    for (uint8_t channel = 1; channel <= 2; ++channel) {
        const uint8_t index = pulseIndex(channel);
        if (context->pulseConfiguration[index].owner != this) continue;
        IO::digitalWritePulse(index, LOW);
        IO::pinModePulse(index, INPUT);
        context->pulseConfiguration[index].owner = nullptr;
    }
    _active = false;
}

inline bool Relay::set(uint8_t relayChannel, bool active) {
    if (!_active || relayChannel < 1 || relayChannel > 2) return false;
    const uint8_t index = pulseIndex(relayChannel);
    auto* context = Context::getInstance();
    if (context->pulseConfiguration[index].owner != this ||
        context->pulseConfiguration[index].pinMode != OUTPUT) return false;
    IO::setShiftRegisterBit(shiftRegisterBit(relayChannel), active ? HIGH : LOW);
    return true;
}

} // namespace VMCBuddy
#endif // RELAY_H
