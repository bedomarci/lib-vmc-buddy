#ifndef IPERIPHERAL_H
#define IPERIPHERAL_H

#define _TASK_STD_FUNCTION
#define _TASK_INLINE
#include <TaskScheduler.h>

namespace VMCBuddy {

class Context;

class IPeripheral : public Task {
public:
    virtual ~IPeripheral() = default;

    /** Register self with the scheduler. Called by VMCBuddy::addPeripheral().
     *  Override to add one or more tasks via ctx->scheduler()->addTask(). */
    virtual void boot(Context* ctx) { ctx->scheduler()->addTask(*this); }

    virtual bool begin() = 0;
    virtual void end() = 0;
    virtual const char* name() const = 0;

protected:
    bool _active = false;
};

} // namespace VMCBuddy

#endif // IPERIPHERAL_H