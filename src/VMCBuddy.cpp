#include "VMCBuddy.h"


#include <TaskScheduler.h>
#include <AnalogButtonReader.h>
#include <PulseReader.h>
#include <LedBlinker.h>
#include <PulseWriter.h>

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
        setLed(ledIndex, LOW);
        auto* led = new LedBlinker(this, scheduler, ledIndex, numberOfBlinks, onTime, offTime);
        led->enableDelayed(delay);
    }

    void VMCBuddy::sendPulse(uint8_t pinIndex, int numberOfPulse, int onTime, int offTime)
    {
        auto* pulse = new PulseWriter(this, scheduler, pinIndex, numberOfPulse, onTime, offTime);
        pulse->enable();
    }

    // Initialization method
    void VMCBuddy::begin()
    {

        for (const auto& it : pinConfiguration)
        {
            pinMode(it.first, it.second);
        }

        for (uint8_t pin : ledPins)
            setLed(pin, LOW);


        this->scheduler->addTask(*this->pulseReader);
        this->scheduler->addTask(*this->analogButtonReader);
        this->analogButtonReader->enable();
        this->pulseReader->enable();
        initialized = true;
    }


    void VMCBuddy::render()
    {
        digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, LOW);
        // Loop through the 16 bits
        for (unsigned char shiftRegisterBit : shiftRegisterBits)
        {
            // Write the bit to the data pin
            digitalWrite(PIN_MCU_SHIFTREGISTER_DATA, shiftRegisterBit ? HIGH : LOW);
            // Pulse the clock to shift the bit
            digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, HIGH);
            digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, LOW);
        }
        // End transmission: Pull latch high
        digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, HIGH);
    }


    void VMCBuddy::setPulseInterruptMode(uint8_t index, const uint8_t mode, uint16_t debounce)
    {
        index = constrain(index, 1, NUM_PULSE) - 1;
        if (initialized)
        {
            Log.errorln("Interrupts must be registered before initialiyation. Move setInterruptMode() above begin(). ");
            return;
        }

        if (pulsePinMode[index] != INPUT)
        {
            Log.errorln("Pulse line is not an INPUT. Change configuration!");
            return;
        }
        PulseReader::setInterruptMode(index, mode, debounce);
    }


    void VMCBuddy::setShiftRegisterBit(uint8_t index, uint8_t value)
    {
        this->shiftRegisterBits[index] = value;
        this->render();
    }

    void VMCBuddy::setLed(uint8_t index, uint8_t value)
    {
        index = constrain(index, 1, NUM_LEDS) - 1;
        this->setShiftRegisterBit(this->ledPins[index], !value);
    }

    void VMCBuddy::digitalWritePulsePin(uint8_t index, uint8_t value)
    {
        index = constrain(index, 1, NUM_PULSE) - 1;
        if (pulsePinMode[index] == INPUT) return;
        this->setShiftRegisterBit(this->pulseOutPins[index], value);
    }

    void VMCBuddy::pinModePulse(uint8_t index, uint8_t mode)
    {
        index = constrain(index, 1, NUM_PULSE) - 1;
        pulsePinMode[index] = mode;

        if (mode == INPUT)
        {
            setShiftRegisterBit(pulseOutPins[index], 0);
        }
    }
}
