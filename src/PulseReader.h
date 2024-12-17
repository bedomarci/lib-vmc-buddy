#ifndef PULSEREADER_H
#define PULSEREADER_H

#include <EventHandler.h>
#include <TaskScheduler.h>
#include <VMCBuddy.h>
#include <IO.h>

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
        void static handleInterrupts(uint8_t pulseIndex);
    };

    inline PulseReader::PulseReader() : Task()
    {
        setInterval(TASK_IMMEDIATE);
        setIterations(TASK_FOREVER);
        setCallback([]() { PulseReader::checkDirty(); });
        setOnEnable([this]()-> bool { return PulseReader::registerInterrupts(); });
        // Context::getInstance()->pulseConfiguration[0] = Pulse(0,PIN_MCU_PULSE_IN1,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[1] = Pulse(1,PIN_MCU_PULSE_IN2,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[2] = Pulse(2,PIN_MCU_PULSE_IN3,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[3] = Pulse(3,PIN_MCU_PULSE_IN4,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[4] = Pulse(4,PIN_MCU_PULSE_IN5,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[5] = Pulse(5,PIN_MCU_PULSE_IN6,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[6] = Pulse(6,PIN_MCU_PULSE_IN7,DEFAULT_ISR_MODE);
        // Context::getInstance()->pulseConfiguration[7] = Pulse(7,PIN_MCU_PULSE_IN8,DEFAULT_ISR_MODE);
    }

    inline void PulseReader::setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce)
    {
        Log.verboseln("Interrupt set for %d", index+1);
        Context::getInstance()->pulseConfiguration[index].interruptMode = mode;
        Context::getInstance()->pulseConfiguration[index].debounce = debounce;
    }

    inline bool PulseReader::registerInterrupts()
    {
        if (Context::getInstance()->pulseConfiguration[0].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[0].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(0); },
                            Context::getInstance()->pulseConfiguration[0].interruptMode);
        if (Context::getInstance()->pulseConfiguration[1].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[1].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(1); },
                            Context::getInstance()->pulseConfiguration[1].interruptMode);
        if (Context::getInstance()->pulseConfiguration[2].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[2].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(2); },
                            Context::getInstance()->pulseConfiguration[2].interruptMode);
        if (Context::getInstance()->pulseConfiguration[3].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[3].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(3); },
                            Context::getInstance()->pulseConfiguration[3].interruptMode);
        if (Context::getInstance()->pulseConfiguration[4].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[4].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(4); },
                            Context::getInstance()->pulseConfiguration[4].interruptMode);
        if (Context::getInstance()->pulseConfiguration[5].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[5].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(5); },
                            Context::getInstance()->pulseConfiguration[5].interruptMode);
        if (Context::getInstance()->pulseConfiguration[6].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[6].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(6); },
                            Context::getInstance()->pulseConfiguration[6].interruptMode);
        if (Context::getInstance()->pulseConfiguration[7].interruptMode)
            attachInterrupt(Context::getInstance()->pulseConfiguration[7].pin,
                            []()IRAM_ATTR { PulseReader::handleInterrupts(7); },
                            Context::getInstance()->pulseConfiguration[7].interruptMode);
        bool hasInterrupts = false;
        for (const auto pulse : Context::getInstance()->pulseConfiguration)
        {
            hasInterrupts |= (pulse.interruptMode != DETACH_ISR);
        }
        if (hasInterrupts) Log.verboseln("There are interrupts attached on pulse input lines.");
        return hasInterrupts;
    }

    inline void PulseReader::handleInterrupts(const uint8_t pulseIndex)
    {
        if (Context::getInstance()->pulseConfiguration[pulseIndex].pinMode != INPUT) return;
        const unsigned long now = millis();
        const unsigned long delta = now - Context::getInstance()->pulseConfiguration[pulseIndex].lastInterrupt;
        // Log.verboseln("INT %i", pulseIndex+1);
        if (delta > Context::getInstance()->pulseConfiguration[pulseIndex].debounce)
        {
            Context::getInstance()->pulseConfiguration[pulseIndex].value = digitalRead(
                Context::getInstance()->pulseConfiguration[pulseIndex].pin);
            Context::getInstance()->pulseConfiguration[pulseIndex].isDirty = true;
            Context::getInstance()->pulseConfiguration[pulseIndex].lastInterrupt = now;
        }
    }

    inline void PulseReader::checkDirty()
    {
        for (int i = 0; i < LEN_PIN_INTERRUPTS; i++)
        {
            if (Context::getInstance()->pulseConfiguration[i].isDirty)
            {
                EventHandler::trigger(Event::PULSE_INT, Context::getInstance()->pulseConfiguration[i].index,
                                      Context::getInstance()->pulseConfiguration[i].value,
                                      Context::getInstance()->pulseConfiguration[i].interruptMode);
                // Log.verboseln("Interrupt dirty at \t%i", Context::getInstance()->pulseConfiguration[i].index);
                Context::getInstance()->pulseConfiguration[i].isDirty = false;
            }
        }
    }
}

#endif //PULSEREADER_H
