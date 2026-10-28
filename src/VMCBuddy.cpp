#include "VMCBuddy.h"
#include <TaskScheduler.h>

namespace VMCBuddy {

VMCBuddy::VMCBuddy(Scheduler* scheduler, Print* serial)
    : _scheduler(scheduler), _serial(serial) {
    if (!_serial) _serial = &Serial;
    Log.begin(LOG_LEVEL_VERBOSE, _serial);
    auto* ctx = Context::getInstance();
    ctx->setScheduler(scheduler);
    ctx->setBoard(this);
}

void VMCBuddy::addPeripheral(IPeripheral* p) {
    _peripherals.push_back(p);
    p->boot(Context::getInstance());
}

void VMCBuddy::begin() {
    const auto& cfg = Context::getInstance()->boardConfig;
    pinMode(cfg.srData, OUTPUT);
    pinMode(cfg.srClock, OUTPUT);
    pinMode(cfg.srLatch, OUTPUT);
    pinMode(cfg.btnAdc, INPUT);
    for (uint8_t i = 0; i < cfg.numPulses; ++i)
        pinMode(cfg.pulseInGpio[i], INPUT);

    for (auto* p : _peripherals) {
        if (p->begin()) Log.infoln("Peripheral started: %s", p->name());
        else Log.errorln("Peripheral failed to start: %s", p->name());
    }

    Log.infoln("VMCBuddy initialized with %d peripherals.", _peripherals.size());
    _initialized = true;
}

} // namespace VMCBuddy