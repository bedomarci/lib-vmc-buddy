#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h> // Arduino framework compatibility
#include <macros.h>
#include <map>

namespace VMCBuddy
{
    struct Pulse
    {
        uint8_t index;
        uint8_t pin;
        uint8_t interruptMode;
        uint8_t pinMode;
        uint8_t value;
        bool isDirty = false;
        unsigned long lastInterrupt = 0;
        uint16_t debounce;

        // Constructor
        Pulse(uint8_t index = 0, uint8_t pin = 0, uint8_t interruptMode = DEFAULT_ISR_MODE,
              uint16_t debounce = DEFAULT_PULSE_DEBOUNCE)
            : index(index), pin(pin), interruptMode(interruptMode), debounce(debounce)
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
        Pulse pulseConfiguration[LEN_PIN_INTERRUPTS];
        // Deleted copy constructor and assignment operator to prevent copying
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        // Provides access to the singleton instance
        static Context* getInstance();

        bool initialized = false;

        uint8_t shiftRegisterBits[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        std::map<uint8_t, uint8_t> pinConfiguration = {
            {PIN_MCU_SHIFTREGISTER_DATA, OUTPUT}, // 2 = Shift Register Data
            {PIN_MCU_SHIFTREGISTER_CLOCK, OUTPUT}, // 3 = Shift Register Clock
            {PIN_MCU_SHIFTREGISTER_LATCH, OUTPUT}, // 4 = Shift Register Latch
            {PIN_MCU_ANALOG_BTNS, INPUT},
            {PIN_MCU_PULSE_IN1, INPUT},
            {PIN_MCU_PULSE_IN2, INPUT},
            {PIN_MCU_PULSE_IN3, INPUT},
            {PIN_MCU_PULSE_IN4, INPUT},
            {PIN_MCU_PULSE_IN5, INPUT},
            {PIN_MCU_PULSE_IN6, INPUT},
            {PIN_MCU_PULSE_IN7, INPUT},
            {PIN_MCU_PULSE_IN8, INPUT},
        };

        const uint8_t ledPins[NUM_LEDS] = {
            PIN_SR_LED1,
            PIN_SR_LED2,
            PIN_SR_LED3,
            PIN_SR_LED4,
            PIN_SR_LED5,
            PIN_SR_LED6,
        };

        const uint8_t pulseOutPins[NUM_PULSE] = {
            PIN_SR_PULSE_OUT1,
            PIN_SR_PULSE_OUT2,
            PIN_SR_PULSE_OUT3,
            PIN_SR_PULSE_OUT4,
            PIN_SR_PULSE_OUT5,
            PIN_SR_PULSE_OUT6,
            PIN_SR_PULSE_OUT7,
            PIN_SR_PULSE_OUT8,
        };

        const uint8_t pulseInPins[NUM_PULSE] = {
            PIN_MCU_PULSE_IN1,
            PIN_MCU_PULSE_IN2,
            PIN_MCU_PULSE_IN3,
            PIN_MCU_PULSE_IN4,
            PIN_MCU_PULSE_IN5,
            PIN_MCU_PULSE_IN6,
            PIN_MCU_PULSE_IN7,
            PIN_MCU_PULSE_IN8,
        };
    };

    inline Context::Context()
    {
        pulseConfiguration[0] = Pulse(0,PIN_MCU_PULSE_IN1,DEFAULT_ISR_MODE);
        pulseConfiguration[1] = Pulse(1,PIN_MCU_PULSE_IN2,DEFAULT_ISR_MODE);
        pulseConfiguration[2] = Pulse(2,PIN_MCU_PULSE_IN3,DEFAULT_ISR_MODE);
        pulseConfiguration[3] = Pulse(3,PIN_MCU_PULSE_IN4,DEFAULT_ISR_MODE);
        pulseConfiguration[4] = Pulse(4,PIN_MCU_PULSE_IN5,DEFAULT_ISR_MODE);
        pulseConfiguration[5] = Pulse(5,PIN_MCU_PULSE_IN6,DEFAULT_ISR_MODE);
        pulseConfiguration[6] = Pulse(6,PIN_MCU_PULSE_IN7,DEFAULT_ISR_MODE);
        pulseConfiguration[7] = Pulse(7,PIN_MCU_PULSE_IN8,DEFAULT_ISR_MODE);
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
