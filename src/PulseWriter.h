#ifndef PulseWriter_H
#define PulseWriter_H
#include <TaskScheduler.h>
#include <VMCBuddy.h>

namespace VMCBuddy
{
    class PulseWriter : public Task
    {
    public:
        PulseWriter(VMCBuddy* board, Scheduler* scheduler, uint8_t pinIndex, uint16_t numberOfPulse, uint16_t onTime,
                    uint16_t offTime);
        ~PulseWriter() override;

    protected:
        void turnOn();
        void turnOff();
        virtual void writeOut(uint8_t state);
        uint16_t numberOfPulse;
        uint16_t onTime;
        uint16_t offTime;
        uint8_t pinIndex;
        TaskCallback onCallback = [this]() { turnOn(); };
        TaskCallback offCallback = [this]() { turnOff(); };

        VMCBuddy* board;
        Scheduler* scheduler;
    };

    inline PulseWriter::PulseWriter(VMCBuddy* board, Scheduler* scheduler, uint8_t pinIndex, uint16_t numberOfPulse,
                                    uint16_t onTime, uint16_t offTime) :
        Task(),
        numberOfPulse(numberOfPulse), onTime(onTime), offTime(offTime),
        pinIndex(pinIndex), board(board), scheduler(scheduler)
    {
        setIterations(numberOfPulse * 2);
        setCallback(onCallback);
        scheduler->addTask(*this);
        // Log.verbosenl("Blink %i times on led #%i", numberOfBlinks, ledIndex);
    }

    inline PulseWriter::~PulseWriter()
    {
        scheduler->deleteTask(*this);
    }

    inline void PulseWriter::turnOn()
    {
        writeOut(HIGH);
        setInterval(onTime);
        setCallback(offCallback);
    }

    inline void PulseWriter::turnOff()
    {
        writeOut(LOW);
        setInterval(offTime);
        setCallback(onCallback);
        if (isLastIteration())
            delete(this);
    }

    inline void PulseWriter::writeOut(uint8_t state)
    {
        board->digitalWritePulsePin(pinIndex, state);
    }
} // VMCBuddy

#endif //PulseWriter_H
