#include "VMCBuddy.h"
#include <TaskScheduler.h>
#include <PulseReader.h>
#include <IO.h>

namespace VMCBuddy {

VMCBuddy::VMCBuddy(Scheduler* scheduler, Print* serial)
    : _scheduler(scheduler), _serial(serial) {
    if (!_serial) _serial = &Serial;
    Log.begin(LOG_LEVEL_VERBOSE, _serial);
}

void VMCBuddy::addPeripheral(IPeripheral* p) {
    _peripherals.push_back(p);
    _scheduler->addTask(*p);
}

void VMCBuddy::begin() {
    // base I/O pin setup
    for (const auto& it : Context::getInstance()->pinConfiguration)
        pinMode(it.first, it.second);

    // begin all registered peripherals
    for (auto* p : _peripherals) {
        if (p->begin()) Log.infoln("Peripheral started: %s", p->name());
        else Log.errorln("Peripheral failed to start: %s", p->name());
    }

    Log.infoln("VMCBuddy initialized with %d peripherals.", _peripherals.size());
    _initialized = true;
}

void VMCBuddy::setPulseInterruptMode(uint8_t index, const uint8_t mode, uint16_t debounce) {
    if (_initialized) {
        Log.errorln("Interrupts must be registered before begin().");
        return;
    }
    if (Context::getInstance()->pulseConfiguration[index].pinMode != INPUT) {
        Log.errorln("Pulse line %i is not an INPUT.", index + 1);
        return;
    }
    PulseReader::setInterruptMode(index, mode, debounce);
}

void VMCBuddy::pinModePulse(uint8_t index, uint8_t mode) { IO::pinModePulse(index, mode); }
void VMCBuddy::digitalWritePulsePin(uint8_t index, uint8_t value) { IO::digitalWritePulse(index, value); }

} // namespace VMCBuddy
