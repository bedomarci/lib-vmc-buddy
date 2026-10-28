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
    const auto& cfg = Context::getInstance()->boardConfig;
    pinMode(cfg.srData, OUTPUT);
    pinMode(cfg.srClock, OUTPUT);
    pinMode(cfg.srLatch, OUTPUT);
    pinMode(cfg.btnAdc, INPUT);
    for (uint8_t i = 0; i < cfg.numPulses; ++i)
        pinMode(cfg.pulseInGpio[i], INPUT);

    // begin all registered peripherals
    for (auto* p : _peripherals) {
        if (p->begin()) Log.infoln("Peripheral started: %s", p->name());
        else Log.errorln("Peripheral failed to start: %s", p->name());
    }

    Log.infoln("VMCBuddy initialized with %d peripherals.", _peripherals.size());
    _initialized = true;
}

void VMCBuddy::setPulseInterruptMode(uint8_t index, const uint8_t mode, uint16_t debounce) {
    auto* context = Context::getInstance();
    if (_initialized) {
        Log.errorln("Interrupts must be registered before begin().");
        return;
    }
    if (index >= context->boardConfig.numPulses ||
        context->pulseConfiguration[index].pinMode != INPUT) {
        Log.errorln("Pulse line %i is not an INPUT.", index + 1);
        return;
    }
    PulseReader::setInterruptMode(index, mode, debounce);
}

void VMCBuddy::pinModePulse(uint8_t index, uint8_t mode) { IO::pinModePulse(index, mode); }
void VMCBuddy::digitalWritePulsePin(uint8_t index, uint8_t value) { IO::digitalWritePulse(index, value); }

} // namespace VMCBuddy
