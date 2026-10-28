#ifndef LED_H
#define LED_H

#include <IPeripheral.h>
#include <PulseWriter.h>

namespace VMCBuddy {

/** Dedicated controller for the six active-low onboard LEDs. */
class Led : public IPeripheral, protected PulseWriter {
public:
    Led();
    bool begin() override;
    void end() override;
    const char* name() const override { return "OnboardLEDs"; }

    bool set(uint8_t ledIndex, uint8_t state);
    bool blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
               uint16_t offTimeMs, uint16_t startDelayMs = 0);

protected:
    bool validTarget(uint8_t index) const override {
        return index < Context::getInstance()->boardConfig.numLeds;
    }
    void writeOut(uint8_t index, uint8_t state) override;

private:
    void tick();
};

inline Led::Led() : IPeripheral(), PulseWriter() {
    setInterval(TASK_IMMEDIATE);
    setIterations(TASK_FOREVER);
    setCallback([this]() { tick(); });
}

inline bool Led::begin() {
    if (_active) return true;
    PulseWriter::startWriter();
    _active = true;
    const uint8_t ledCount = Context::getInstance()->boardConfig.numLeds;
    for (uint8_t i = 0; i < ledCount; ++i) set(i, LOW);
    return true;
}

inline void Led::end() {
    Task::disable();
    PulseWriter::stopWriter();
    _active = false;
}

inline bool Led::set(uint8_t ledIndex, uint8_t state) {
    if (!validTarget(ledIndex)) return false;
    writeOut(ledIndex, state);
    return true;
}

inline bool Led::blink(uint8_t ledIndex, uint16_t count, uint16_t onTimeMs,
                       uint16_t offTimeMs, uint16_t startDelayMs) {
    if (!_active || !PulseWriter::startPulseTrain(ledIndex, count, onTimeMs,
                                                  offTimeMs, startDelayMs)) return false;
    Task::disable();
    setInterval(PulseWriter::writerDelayMs());
    Task::enable();
    return true;
}

inline void Led::writeOut(uint8_t index, uint8_t state) {
    const uint8_t bit = Context::getInstance()->boardConfig.ledSrBit[index];
    IO::setShiftRegisterBit(bit, state ? LOW : HIGH); // active-low LED
}

inline void Led::tick() {
    PulseWriter::updateWriter();
    if (PulseWriter::writerBusy()) setInterval(PulseWriter::writerDelayMs());
    else Task::disable();
}

} // namespace VMCBuddy
#endif // LED_H
