#ifndef IO_H
#define IO_H

#include <Context.h>

namespace VMCBuddy
{
    class IO
    {
    public:
        static void digitalWritePulse(uint8_t index, uint8_t state);
        static void pinModePulse(uint8_t index, uint8_t mode);
        static void setShiftRegisterBit(uint8_t index, uint8_t value);
        static void setLed(uint8_t index, uint8_t value);
        static void render();
    private:
    };

    inline void IO::pinModePulse(uint8_t index, uint8_t mode)
    {
        index = constrain(index, 1, NUM_PULSE) - 1;
        Context::getInstance()->pulseConfiguration[index] = mode;

        if (mode == INPUT)
        {
            IO::setShiftRegisterBit(Context::getInstance()->pulseOutPins[index], 0);
        }
    }

    inline void IO::digitalWritePulse(uint8_t index, uint8_t state)
    {
        index = constrain(index, 1, NUM_PULSE) - 1;
        if (Context::getInstance()->pulseConfiguration[index].pinMode == INPUT)
        {
            Log.errorln("Pin #%i is in INPUT mode. You can not write out!", index + 1);
            return;
        }
        setShiftRegisterBit(Context::getInstance()->pulseConfiguration[index].pin, state);
    }

    inline void IO::setShiftRegisterBit(uint8_t index, uint8_t value)
    {
        Context::getInstance()->shiftRegisterBits[index] = value;
        render();
    }

    inline void IO::setLed(uint8_t index, uint8_t value)
    {
        index = constrain(index, 1, NUM_LEDS) - 1;
        setShiftRegisterBit(Context::getInstance()->ledPins[index], !value);
    }

    inline void IO::render()
    {
        digitalWritePulse(PIN_MCU_SHIFTREGISTER_LATCH, LOW);
        // Loop through the 16 bits
        for (unsigned char shiftRegisterBit : Context::getInstance()->shiftRegisterBits)
        {
            // Write the bit to the data pin
            digitalWritePulse(PIN_MCU_SHIFTREGISTER_DATA, shiftRegisterBit ? HIGH : LOW);
            // Pulse the clock to shift the bit
            digitalWritePulse(PIN_MCU_SHIFTREGISTER_CLOCK, HIGH);
            digitalWritePulse(PIN_MCU_SHIFTREGISTER_CLOCK, LOW);
        }
        // End transmission: Pull latch high
        digitalWritePulse(PIN_MCU_SHIFTREGISTER_LATCH, HIGH);
    }
} // VMCBuddy

#endif //IO_H
