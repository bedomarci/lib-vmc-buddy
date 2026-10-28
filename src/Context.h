#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h> // Arduino framework compatibility
#include <macros.h>

namespace VMCBuddy
{
    struct PulseChannel
    {
        uint8_t index;
        uint8_t interruptMode = DETACH_ISR;
        uint8_t pinMode = INPUT;
        uint8_t value = LOW;
        bool isDirty = false;
        unsigned long lastInterrupt = 0;
        uint16_t debounce = DEFAULT_PULSE_DEBOUNCE;

        explicit PulseChannel(uint8_t index = 0, uint8_t interruptMode = DETACH_ISR,
                              uint16_t debounce = DEFAULT_PULSE_DEBOUNCE)
            : index(index), interruptMode(interruptMode), debounce(debounce)
        {
        }
    };

    class Context
    {
    private:
        // Private constructor to prevent instantiation
        Context();

        // Destructor
        ~Context() = default;

        static Context* instance;

    public:
        BoardConfig boardConfig;
        PulseChannel pulseConfiguration[LEN_PIN_INTERRUPTS];
        // Deleted copy constructor and assignment operator to prevent copying
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        // Provides access to the singleton instance
        static Context* getInstance();

        bool initialized = false;

        uint8_t shiftRegisterBits[LEN_SHIFT_REGISTER_BITS] = {};
    };

    inline Context::Context() : boardConfig(makeBoardConfig())
    {
        for (uint8_t i = 0; i < boardConfig.numPulses; ++i)
            pulseConfiguration[i] = PulseChannel(i, boardConfig.detachIsr,
                                                 boardConfig.defaultPulseDebounce);
    }

    inline Context* Context::getInstance()
    {
        if (!instance)
        {
            instance = new Context();
        }
        return instance;
    }
} // VMCBuddy

#endif //CONTEXT_H
