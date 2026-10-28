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

    void addPeripheral(IPeripheral* p);
    void begin();

    Scheduler* scheduler() { return _scheduler; }

private:
    Scheduler* _scheduler;
    Print* _serial;
    bool _initialized = false;
    std::vector<IPeripheral*> _peripherals;
};

} // namespace VMCBuddy
#endif // VMC_BUDDY_H