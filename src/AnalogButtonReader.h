#ifndef ANALOGBUTTONREADER_H
#define ANALOGBUTTONREADER_H

#include <IPeripheral.h>
#include <EventHandler.h>
#include <macros.h>

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

    const int analogButtonValues[NUM_BUTTONS] = {
        VAL_ANALOG_BTN1, VAL_ANALOG_BTN2, VAL_ANALOG_BTN3,
        VAL_ANALOG_BTN4, VAL_ANALOG_BTN5, VAL_ANALOG_BTN6,
    };
};

inline AnalogButtonReader::AnalogButtonReader() : IPeripheral() {
    setInterval(TIME_ANALOG_BUTTON_READ_INTERVAL);
    setIterations(TASK_FOREVER);
    setCallback([this]() { this->readAnalogButtons(); });
}

inline bool AnalogButtonReader::begin() { Task::enable(); _active = true; return true; }
inline void AnalogButtonReader::end() { Task::disable(); _active = false; }

inline void AnalogButtonReader::readAnalogButtons() {
    newAnalogButtonReadValue = analogRead(PIN_MCU_ANALOG_BTNS);
    int delta = abs(newAnalogButtonReadValue - oldAnalogButtonReadValue);
    if (delta > TOLERANCE_ANALOG_BUTTONS_CHANGE) {
        if (newAnalogButtonReadValue < TOLERANCE_ANALOG_BUTTONS_CHANGE) {
            EventHandler::trigger(Event::BUTTON_RELEASE, -1);
        } else {
            for (int i = 0; i < NUM_BUTTONS; i++) {
                if (newAnalogButtonReadValue > analogButtonValues[i] - TOLERANCE_ANALOG_BUTTONS_CHANGE &&
                    newAnalogButtonReadValue < analogButtonValues[i] + TOLERANCE_ANALOG_BUTTONS_CHANGE) {
                    EventHandler::trigger(Event::BUTTON_PRESS, i);
                }
            }
        }
    }
    oldAnalogButtonReadValue = newAnalogButtonReadValue;
}

} // namespace VMCBuddy
#endif // ANALOGBUTTONREADER_H