#ifndef RELAY_H
#define RELAY_H

#include <IPeripheral.h>
#include <RelayChannel.h>

namespace VMCBuddy {

class Relay : public IPeripheral {
public:
    Relay();

    void boot(Context* ctx) override;
    bool begin() override;
    void end() override;
    const char* name() const override { return "Relay"; }

    bool set(uint8_t relayChannel, bool active);

    RelayChannel* channel(uint8_t index) {
        return index < 2 ? _channels[index] : nullptr;
    }

private:
    static uint8_t relaySrBit(uint8_t relayChannel) {
        return relayChannel == 1 ? PIN_SR_RELAY1 : PIN_SR_RELAY2;
    }
    static uint8_t relayBoardIndex(uint8_t relayChannel) {
        return static_cast<uint8_t>(NUM_PULSE - 2 + relayChannel - 1);
    }

    RelayChannel* _channels[2] = {};
};

inline Relay::Relay() : IPeripheral() { Task::disable(); }

inline void Relay::boot(Context* ctx) {
    for (uint8_t i = 0; i < 2; ++i) {
        char nameBuf[10];
        snprintf(nameBuf, sizeof(nameBuf), "Relay%u", i + 1);
        auto* ch = new RelayChannel(nameBuf, relayBoardIndex(i + 1), relaySrBit(i + 1));
        _channels[i] = ch;
        ch->boot(ctx);
    }
}

inline bool Relay::begin() {
    if (_active) return true;
    _active = true;
    for (uint8_t i = 0; i < 2; ++i)
        if (_channels[i]) _channels[i]->begin();
    return true;
}

inline void Relay::end() {
    _active = false;
    for (auto* ch : _channels) {
        if (ch) {
            ch->end();
            delete ch;
        }
    }
}

inline bool Relay::set(uint8_t relayChannel, bool active) {
    if (!_active || relayChannel < 1 || relayChannel > 2) return false;
    auto* ch = channel(relayChannel - 1);
    if (!ch || !ch->isOutput()) return false;
    ch->write(active ? HIGH : LOW);
    return true;
}

} // namespace VMCBuddy
#endif // RELAY_H