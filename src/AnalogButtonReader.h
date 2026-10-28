#ifndef ANALOGBUTTONREADER_H
#define ANALOGBUTTONREADER_H

#include <IPeripheral.h>
#include <EventHandler.h>
#include <Context.h>

namespace VMCBuddy {

/**
 * Analog button reader — 6-button resistor ladder on a single ADC pin.
 *
 * Polls the ADC every 200 ms with configurable hysteresis tolerance.
 * Dispatches BUTTON_PRESS / BUTTON_RELEASE events when the ladder
 * voltage crosses a threshold.
 *
 * Inherits IPeripheral → Task for scheduler integration.
 */
class AnalogButtonReader : public IPeripheral {
public:
    AnalogButtonReader();
    bool begin() override;
    void end() override;
    const char* name() const override { return "AnalogBtns"; }

protected:
    void readAnalogButtons();
    int oldAnalogButtonReadValue = 0;
    int newAnalogButtonReadValue = 0;

};

inline AnalogButtonReader::AnalogButtonReader() : IPeripheral() {
    setInterval(Context::getInstance()->boardConfig.btnReadIntervalMs);
    setIterations(TASK_FOREVER);
    setCallback([this]() { this->readAnalogButtons(); });
}

inline bool AnalogButtonReader::begin() { Task::enable(); _active = true; return true; }
inline void AnalogButtonReader::end() { Task::disable(); _active = false; }

inline void AnalogButtonReader::readAnalogButtons() {
    const auto& cfg = Context::getInstance()->boardConfig;
    newAnalogButtonReadValue = analogRead(cfg.btnAdc);
    int delta = abs(newAnalogButtonReadValue - oldAnalogButtonReadValue);
    if (delta > cfg.btnTolerance) {
        if (newAnalogButtonReadValue < cfg.btnTolerance) {
            EventHandler::trigger(Event::BUTTON_RELEASE, -1);
        } else {
            for (uint8_t i = 0; i < cfg.numButtons; i++) {
                if (newAnalogButtonReadValue > cfg.btnThresholds[i] - cfg.btnTolerance &&
                    newAnalogButtonReadValue < cfg.btnThresholds[i] + cfg.btnTolerance) {
                    EventHandler::trigger(Event::BUTTON_PRESS, i);
                }
            }
        }
    }
    oldAnalogButtonReadValue = newAnalogButtonReadValue;
}

} // namespace VMCBuddy
#endif // ANALOGBUTTONREADER_H
