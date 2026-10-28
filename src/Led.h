#ifndef LED_H
#define LED_H

#include <IPeripheral.h>
#include <LedChannel.h>

namespace VMCBuddy {

class Led : public IPeripheral {
public:
    Led();

    void boot(Context* ctx) override;
    bool begin() override;
    void end() override;
    const char* name() const override { return "Led"; }

    bool set(uint8_t ledIndex, uint8_t state);
    bool blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
               uint16_t offTimeMs, uint16_t startDelayMs = 0);

    LedChannel* channel(uint8_t index) {
        return index < Context::getInstance()->boardConfig.numLeds ? _channels[index] : nullptr;
    }

private:
    LedChannel* _channels[NUM_LEDS] = {};
};

inline Led::Led() : IPeripheral() { Task::disable(); }

inline void Led::boot(Context* ctx) {
    const auto& cfg = Context::getInstance()->boardConfig;
    char nameBuf[8];
    for (uint8_t i = 0; i < cfg.numLeds; ++i) {
        snprintf(nameBuf, sizeof(nameBuf), "Led%u", i);
        auto* ch = new LedChannel(nameBuf, i, cfg.ledSrBit[i]);
        _channels[i] = ch;
        ch->boot(ctx);
    }
}

inline bool Led::begin() {
    if (_active) return true;
    _active = true;
    for (uint8_t i = 0; i < Context::getInstance()->boardConfig.numLeds; ++i)
        if (_channels[i]) _channels[i]->begin();
    return true;
}

inline void Led::end() {
    _active = false;
    for (auto* ch : _channels) {
        if (ch) {
            ch->end();
            delete ch;
        }
    }
}

inline bool Led::set(uint8_t ledIndex, uint8_t state) {
    auto* ch = channel(ledIndex);
    return ch && ch->set(state);
}

inline bool Led::blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
                       uint16_t offTimeMs, uint16_t startDelayMs) {
    auto* ch = channel(ledIndex);
    return ch && ch->blink(count, onTimeMs, offTimeMs, startDelayMs);
}

} // namespace VMCBuddy
#endif // LED_H