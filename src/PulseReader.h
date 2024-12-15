#ifndef PULSEREADER_H
#define PULSEREADER_H

#include <EventHandler.h>
#include <TaskScheduler.h>
#include <VMCBuddy.h>

namespace VMCBuddy
{
    class PulseReader : public Task
    {
    public:
        PulseReader();
        static void setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);
        static void checkDirty();
        static bool registerInterrupts();

    protected:
        struct Pulse
        {
            uint8_t index;
            uint8_t pin;
            uint8_t interruptMode;
            uint8_t value;
            bool isDirty = false;
            unsigned long lastInterrupt = 0;
            uint16_t debounce;

            // Constructor
            Pulse(uint8_t index = 0, uint8_t pin = 0, uint8_t interruptMode = 0,
                  uint16_t debounce = DEFAULT_PULSE_DEBOUNCE)
                : index(index), pin(pin), interruptMode(interruptMode), debounce(debounce)
            {
            }
        };

        static Pulse pulseBuffer[LEN_PIN_INTERRUPTS];

        void static handleInterrupts(uint8_t pulseIndex);
    };

    inline PulseReader::PulseReader() : Task()
    {
        setInterval(TASK_IMMEDIATE);
        setIterations(TASK_FOREVER);
        setCallback([]() { PulseReader::checkDirty(); });
        setOnEnable([this]()-> bool { return PulseReader::registerInterrupts(); });
        PulseReader::pulseBuffer[0] = Pulse(0,PIN_MCU_PULSE_IN1,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[1] = Pulse(1,PIN_MCU_PULSE_IN2,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[2] = Pulse(2,PIN_MCU_PULSE_IN3,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[3] = Pulse(3,PIN_MCU_PULSE_IN4,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[4] = Pulse(4,PIN_MCU_PULSE_IN5,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[5] = Pulse(5,PIN_MCU_PULSE_IN6,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[6] = Pulse(6,PIN_MCU_PULSE_IN7,DEFAULT_ISR_MODE);
        PulseReader::pulseBuffer[7] = Pulse(7,PIN_MCU_PULSE_IN8,DEFAULT_ISR_MODE);
    }

    PulseReader::Pulse PulseReader::pulseBuffer[LEN_PIN_INTERRUPTS];

    inline void PulseReader::setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce)
    {
        Log.verboseln("Interrupt set for %d", index);
        pulseBuffer[index].interruptMode = mode;
        pulseBuffer[index].debounce = debounce;
    }

    inline bool PulseReader::registerInterrupts()
    {
        if (pulseBuffer[0].interruptMode)
            attachInterrupt(pulseBuffer[0].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(0); },
                            pulseBuffer[0].interruptMode);
        if (pulseBuffer[1].interruptMode)
            attachInterrupt(pulseBuffer[1].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(1); },
                            pulseBuffer[1].interruptMode);
        if (pulseBuffer[2].interruptMode)
            attachInterrupt(pulseBuffer[2].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(2); },
                            pulseBuffer[2].interruptMode);
        if (pulseBuffer[3].interruptMode)
            attachInterrupt(pulseBuffer[3].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(3); },
                            pulseBuffer[3].interruptMode);
        if (pulseBuffer[4].interruptMode)
            attachInterrupt(pulseBuffer[4].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(4); },
                            pulseBuffer[4].interruptMode);
        if (pulseBuffer[5].interruptMode)
            attachInterrupt(pulseBuffer[5].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(5); },
                            pulseBuffer[5].interruptMode);
        if (pulseBuffer[6].interruptMode)
            attachInterrupt(pulseBuffer[6].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(6); },
                            pulseBuffer[6].interruptMode);
        if (pulseBuffer[7].interruptMode)
            attachInterrupt(pulseBuffer[7].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(7); },
                            pulseBuffer[7].interruptMode);
        bool hasInterrupts = false;
        for (const auto pulse : pulseBuffer)
        {
            hasInterrupts |= (pulse.interruptMode != DETACH_ISR);
        }
        if (hasInterrupts) Log.verboseln("There are interrupts attached on pulse input lines.");
        return hasInterrupts;
    }

    inline void PulseReader::handleInterrupts(const uint8_t pulseIndex)
    {
        // if (pulseBuffer[pulseIndex].interruptMode != INPUT) return;
        const unsigned long now = millis();
        const unsigned long delta = now - pulseBuffer[pulseIndex].lastInterrupt;
        // Log.verboseln("INT %i", pulseIndex+1);
        if (delta > pulseBuffer[pulseIndex].debounce)
        {
            pulseBuffer[pulseIndex].value = digitalRead(pulseBuffer[pulseIndex].pin);
            pulseBuffer[pulseIndex].isDirty = true;
            pulseBuffer[pulseIndex].lastInterrupt = now;
        }
    }

    inline void PulseReader::checkDirty()
    {
        for (int i = 0; i < LEN_PIN_INTERRUPTS; i++)
        {
            if (pulseBuffer[i].isDirty)
            {
                EventHandler::trigger(Event::PULSE_INT, pulseBuffer[i].index + 1, pulseBuffer[i].value,
                                      pulseBuffer[i].interruptMode);
                Log.verboseln("Interrupt dirty at \t%i", PulseReader::pulseBuffer[i].index+1);
                PulseReader::pulseBuffer[i].isDirty = false;
            }
        }
    }
}

#endif //PULSEREADER_H
