#include "VMCBuddy.h"


#include <TaskScheduler.h>
#include <AnalogButtonReader.h>
#include <PulseReader.h>
#include <LedBlinker.h>
#include <PulseWriter.h>
#include <IO.h>

namespace VMCBuddy
{
    // Constructor
    VMCBuddy::VMCBuddy(Scheduler* scheduler, Print* serial)
        : scheduler(scheduler),
          serial(serial)
    {
        if (!this->serial)
        {
            this->serial = &Serial;
        }
        Log.begin(LOG_LEVEL_VERBOSE, this->serial);
        pulseReader = new PulseReader();
        analogButtonReader = new AnalogButtonReader();
    }

    void VMCBuddy::blink(uint8_t ledIndex, int numberOfBlinks, int onTime, int offTime, int delay)
    {
        IO::setLed(ledIndex, LOW);
        auto* led = new LedBlinker(scheduler, ledIndex, numberOfBlinks, onTime, offTime);
        led->enableDelayed(delay);
    }

    void VMCBuddy::sendPulse(uint8_t pinIndex, int numberOfPulse, int onTime, int offTime)
    {
        auto* pulse = new PulseWriter(scheduler, pinIndex, numberOfPulse, onTime, offTime);
        pulse->enable();
    }

    // Initialization method
    void VMCBuddy::begin()
    {
        for (const auto& it : context()->pinConfiguration)
        {
            pinMode(it.first, it.second);
        }

        for (uint8_t i = 0; i < NUM_LEDS; i++)
            IO::setLed(i, LOW);


        this->scheduler->addTask(*this->pulseReader);
        this->scheduler->addTask(*this->analogButtonReader);
        this->analogButtonReader->enable();
        this->pulseReader->enable();


        context()->initialized = true;
        Log.infoln("VMCBuddy initialized.");
    }


    void VMCBuddy::setPulseInterruptMode(uint8_t index, const uint8_t mode, uint16_t debounce)
    {
        // index = constrain(index, 1, NUM_PULSE) - 1;
        if (context()->initialized)
        {
            Log.errorln("Interrupts must be registered before initialization. Move setInterruptMode() above begin(). ");
            return;
        }

        if (context()->pulseConfiguration[index].pinMode != INPUT)
        {
            Log.errorln("Pulse line %i is not an INPUT. Change configuration!", index+1);
            return;
        }
        PulseReader::setInterruptMode(index, mode, debounce);
    }


    Context* VMCBuddy::context()
    {
        return Context::getInstance();
    }

    // void VMCBuddy::setLed(uint8_t index, uint8_t value)
    // {
    //     index = constrain(index, 1, NUM_LEDS) - 1;
    //     ShiftRegisterWriter::setBit(context()->ledPins[index], !value);
    //     IO
    // }
    //
    // void VMCBuddy::digitalWritePulsePin(uint8_t index, uint8_t value)
    // {
    //     index = constrain(index, 1, NUM_PULSE) - 1;
    //
    // }
    //
    void VMCBuddy::pinModePulse(uint8_t index, uint8_t mode)
    {
        IO::pinModePulse(index, mode);
    }
}
