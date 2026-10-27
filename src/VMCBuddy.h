#ifndef VMC_BUDDY_H
#define VMC_BUDDY_H

#define _TASK_STD_FUNCTION
#define _TASK_INLINE

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <ArduinoLog.h>
#include <macros.h>
#include <Context.h>
#include <IPeripheral.h>
#include <vector>

namespace VMCBuddy {

class VMCBuddy {
public:
    explicit VMCBuddy(Scheduler* scheduler, Print* serial = nullptr);

    /** Register a peripheral. It will be added to the scheduler and
        begin()-ed when VMCBuddy::begin() is called. */
    void addPeripheral(IPeripheral* p);

    /** Call once after registering peripherals. Starts each peripheral. */
    void begin();

    // — pulse I/O configuration —
    void digitalWritePulsePin(uint8_t index, uint8_t value);
    void pinModePulse(uint8_t index, uint8_t mode);
    void setPulseInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);

    void setOnEventCallback(std::function<void(uint8_t, int)> callback);
    Scheduler* scheduler() { return _scheduler; }

private:
    Scheduler* _scheduler;
    Print* _serial;
    bool _initialized = false;
    std::vector<IPeripheral*> _peripherals;
};

} // namespace VMCBuddy
#endif // VMC_BUDDY_H
