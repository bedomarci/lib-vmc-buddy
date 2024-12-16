#ifndef VMC_BUDDY_H
#define VMC_BUDDY_H

#define _TASK_STD_FUNCTION

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <ArduinoLog.h>
#include <macros.h>
#include <Context.h>

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

        // void setLed(uint8_t index, uint8_t value);
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
        // uint8_t shiftRegisterBits[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        //uint8_t pulsePinMode[NUM_PULSE] = {0, 0, 0, 0, 0, 0, 0, 0};
        // std::map<uint8_t,uint8_t> inputValues;
        bool initialized = false;

        AnalogButtonReader* analogButtonReader = nullptr;
        PulseReader* pulseReader = nullptr;

        Context * context();



    };

}

#endif // VMC_BUDDY_H
