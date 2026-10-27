#ifndef IPERIPHERAL_H
#define IPERIPHERAL_H

#define _TASK_STD_FUNCTION
#define _TASK_INLINE
#include <TaskScheduler.h>

namespace VMCBuddy {

/**
 * Base class for every hardware or software peripheral on the board.
 * All peripherals are Tasks — they can be added to the cooperative
 * scheduler and polled automatically.
 *
 * Subclass and override begin(), end(), and name(). The VMCBuddy
 * facade calls begin() on every registered peripheral during its
 * own begin().
 */
class IPeripheral : public Task {
public:
    virtual ~IPeripheral() = default;

    /** One-time hardware setup. Called by VMCBuddy::begin(). */
    virtual bool begin() = 0;
    /** Shut down and release resources. */
    virtual void end() = 0;
    /** Short human-readable identifier for logging / introspection. */
    virtual const char* name() const = 0;

protected:
    bool _active = false;
};

} // namespace VMCBuddy

#endif // IPERIPHERAL_H
