#ifndef VMC_BUDDY_H
#define VMC_BUDDY_H

#define _TASK_STD_FUNCTION

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <ArduinoLog.h>
#include <macros.h>
#include <map>

namespace VMCBuddy
{
    class AnalogButtonReader;
    class PulseReader;
    class VMCBuddy
    {
        // friend class AnalogButtonReader;
    public:
        // Constructor accepting a pointer to the TaskScheduler
        VMCBuddy(Scheduler* scheduler, Print* serial = nullptr);

        void setLed(uint8_t index, uint8_t value);
        void digitalWritePulsePin(uint8_t index, uint8_t value);
        void pinModePulse(uint8_t index, uint8_t mode);
        void setOnEventCallback(std::function<void(uint8_t, int)> callback);
        void blink(uint8_t ledIndex, int numberOfBlinks, int onTime, int offTime, int delay = 0);
        void sendPulse(uint8_t pulseIndex, int numberOfPulse, int onTime, int offTime);

        // void blinkLed();

        // Initialize the library
        void setPulseInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);
        void begin();






    private:
        ::Scheduler* scheduler;
        ::Print* serial;
        // std::function<void(Event, int)> onEventCallback;
        uint8_t shiftRegisterBits[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t pulsePinMode[NUM_PULSE] = {0, 0, 0, 0, 0, 0, 0, 0};
        // std::map<uint8_t,uint8_t> inputValues;
        bool initialized = false;
        bool interruptHappened = false;

        AnalogButtonReader* analogButtonReader = nullptr;
        PulseReader* pulseReader = nullptr;

        // ::Task *tBlinkLed = nullptr;

        void render();
        void readAnalogButtons();
        void setShiftRegisterBit(uint8_t index, uint8_t value);

        // const uint8_t pinConfiguration[LEN_PIN_CONFIGURATION][2] = {
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

}

#endif // VMC_BUDDY_H
