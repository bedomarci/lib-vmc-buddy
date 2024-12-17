#ifndef ANALOGBUTTONREADER_H
#define ANALOGBUTTONREADER_H
#include <EventHandler.h>
#include <TaskScheduler.h>
#include <macros.h>

namespace VMCBuddy
{
    class AnalogButtonReader : public Task
    {
    public:
        explicit AnalogButtonReader() :
            Task(TIME_ANALOG_BUTTON_READ_INTERVAL, TASK_FOREVER, [this]() { this->readAnalogButtons(); })
        {
        }

    protected:
        void readAnalogButtons();

        int oldAnalogButtonReadValue = 0;
        int newAnalogButtonReadValue = 0;

        const int analogButtonValues[NUM_BUTTONS] = {
            VAL_ANALOG_BTN1,
            VAL_ANALOG_BTN2,
            VAL_ANALOG_BTN3,
            VAL_ANALOG_BTN4,
            VAL_ANALOG_BTN5,
            VAL_ANALOG_BTN6,
        };
    };


    inline void AnalogButtonReader::readAnalogButtons()
    {
        this->newAnalogButtonReadValue = analogRead(PIN_MCU_ANALOG_BTNS);
        int delta = abs(this->newAnalogButtonReadValue - this->oldAnalogButtonReadValue);
        if (delta > TOLERANCE_ANALOG_BUTTONS_CHANGE)
        {
            Log.verboseln("Analog button value changed from %i to %i", this->oldAnalogButtonReadValue, this->newAnalogButtonReadValue );
            if (this->newAnalogButtonReadValue < TOLERANCE_ANALOG_BUTTONS_CHANGE)
            {
                EventHandler::trigger(Event::BUTTON_RELEASE, -1);
            }
            else
            {
                for (int i = 0; i < NUM_BUTTONS; i++)
                {
                    if (this->newAnalogButtonReadValue > analogButtonValues[i] -
                        TOLERANCE_ANALOG_BUTTONS_CHANGE
                        &&
                        this->newAnalogButtonReadValue < analogButtonValues[i] +
                        TOLERANCE_ANALOG_BUTTONS_CHANGE)
                    {
                        EventHandler::trigger(Event::BUTTON_PRESS, i);
                    }
                }
            }
        }
        this->oldAnalogButtonReadValue = this->newAnalogButtonReadValue;
    }
}

#endif //ANALOGBUTTONREADER_H
