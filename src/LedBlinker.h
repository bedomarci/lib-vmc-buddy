#ifndef LEDBLINKER_H
#define LEDBLINKER_H
#include <TaskScheduler.h>
#include <VMCBuddy.h>
#include <PulseWriter.h>

namespace VMCBuddy
{
    class LedBlinker : public PulseWriter
    {
    public:
        LedBlinker(VMCBuddy* board, Scheduler* scheduler, uint8_t ledIndex, uint16_t numberOfBlinks, uint16_t onTime,
                   uint16_t offTime);

    protected:
        void writeOut(uint8_t state) override;
    };

    inline LedBlinker::LedBlinker(VMCBuddy* board, Scheduler* scheduler, uint8_t ledIndex, uint16_t numberOfBlinks,
                                  uint16_t onTime, uint16_t offTime) : PulseWriter(
        board, scheduler, ledIndex, numberOfBlinks, onTime, offTime)
    {
    }

    inline void LedBlinker::writeOut(uint8_t state)
    {
        board->setLed(pinIndex, state);
    }
}

#endif //LEDBLINKER_H
