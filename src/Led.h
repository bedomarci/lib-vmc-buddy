#ifndef LED_H
#define LED_H

#include <PulseWriter.h>

namespace VMCBuddy {

/** Dedicated controller for the six onboard feedback LEDs.
 * Inherits PulseWriter's reusable finite pulse task; blink() validates
 * the LED index then starts the same phase-timed pulse sequence. */
class Led : public PulseWriter {
public:
    Led() = default;

    bool begin() override;
    void end() override { PulseWriter::end(); }
    const char* name() const override { return "OnboardLEDs"; }

    bool set(uint8_t ledIndex, uint8_t state);
    bool blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
               uint16_t offTimeMs, uint16_t startDelayMs = 0);

protected:
    bool validTarget(uint8_t index) const override {
        return index < Context::getInstance()->boardConfig.numLeds;
    }
    void writeOut(uint8_t index, uint8_t state) override;
};

inline bool Led::begin() {
    PulseWriter::begin();
    const uint8_t ledCount = Context::getInstance()->boardConfig.numLeds;
    for (uint8_t i = 0; i < ledCount; ++i) set(i, LOW);
    return true;
}

inline bool Led::set(uint8_t ledIndex, uint8_t state) {
    if (!validTarget(ledIndex)) return false;
    writeOut(ledIndex, state);
    return true;
}

inline bool Led::blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
                       uint16_t offTimeMs, uint16_t startDelayMs) {
    if (!validTarget(ledIndex)) return false;
    return sendPulses(ledIndex, count, onTimeMs, offTimeMs, startDelayMs);
}

inline void Led::writeOut(uint8_t index, uint8_t state) {
    const uint8_t bit = Context::getInstance()->boardConfig.ledSrBit[index];
    IO::setShiftRegisterBit(bit, state ? LOW : HIGH); // active-low LED
}

} // namespace VMCBuddy
#endif // LED_H
