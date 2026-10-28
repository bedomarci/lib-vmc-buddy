#ifndef PULSEREADER_H
#define PULSEREADER_H

#include <IPeripheral.h>
#include <EventHandler.h>
#include <VMCBuddy.h>
#include <IO.h>

namespace VMCBuddy {

/**
 * Pulse input reader — optoisolated digital input channels.
 *
 * Each pulse input is optocoupled and wired to an ESP32 GPIO.
 * ISR-based interrupt detection with configurable debounce.
 * Dispatches PULSE_INT events when an input changes state.
 *
 * Inherits IPeripheral → Task for scheduler integration.
 */
class PulseReader : public IPeripheral {
public:
    PulseReader();
    bool begin() override;
    void end() override;
    const char* name() const override { return "PulseReader"; }

    static void setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);
    static void checkDirty();
    static bool registerInterrupts();

protected:
    static void IRAM_ATTR handleInterruptArg(void* arg);
    static void IRAM_ATTR handleInterrupts(uint8_t pulseIndex);
};

inline PulseReader::PulseReader() : IPeripheral() {
    setInterval(TASK_IMMEDIATE);
    setIterations(TASK_FOREVER);
    setCallback([]() { PulseReader::checkDirty(); });
    setOnEnable([this]()-> bool { return PulseReader::registerInterrupts(); });
}

inline bool PulseReader::begin() {
    Task::enable();
    _active = true;
    return true;
}
inline void PulseReader::end() {
    Task::disable();
    _active = false;
}

inline void PulseReader::setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return;
    auto& pulse = context->pulseConfiguration[index];
    pulse.interruptMode = pulse.pinMode == INPUT ? mode : DETACH_ISR;
    pulse.debounce = debounce;
}

inline bool PulseReader::registerInterrupts() {
    auto* context = Context::getInstance();
    bool hasInterrupts = false;
    for (uint8_t i = 0; i < context->boardConfig.numPulses; ++i) {
        auto& pulse = context->pulseConfiguration[i];
        if (pulse.pinMode != INPUT || pulse.interruptMode == DETACH_ISR) {
            detachInterrupt(context->boardConfig.pulseInGpio[i]);
            continue;
        }
        attachInterruptArg(context->boardConfig.pulseInGpio[i], PulseReader::handleInterruptArg,
                           reinterpret_cast<void*>(static_cast<uintptr_t>(i + 1)),
                           pulse.interruptMode);
        hasInterrupts = true;
    }
    return hasInterrupts;
}

inline void IRAM_ATTR PulseReader::handleInterruptArg(void* arg) {
    const uintptr_t encodedIndex = reinterpret_cast<uintptr_t>(arg);
    if (encodedIndex == 0 ||
        encodedIndex > Context::getInstance()->boardConfig.numPulses) return;
    PulseReader::handleInterrupts(static_cast<uint8_t>(encodedIndex - 1));
}

inline void IRAM_ATTR PulseReader::handleInterrupts(const uint8_t pulseIndex) {
    auto* context = Context::getInstance();
    auto& pulse = context->pulseConfiguration[pulseIndex];
    if (pulse.pinMode != INPUT) return;
    const unsigned long now = millis();
    if (now - pulse.lastInterrupt > pulse.debounce) {
        pulse.value = digitalRead(context->boardConfig.pulseInGpio[pulseIndex]);
        pulse.isDirty = true;
        pulse.lastInterrupt = now;
    }
}

inline void PulseReader::checkDirty() {
    auto* context = Context::getInstance();
    for (uint8_t i = 0; i < context->boardConfig.numPulses; i++) {
        if (context->pulseConfiguration[i].isDirty) {
            auto& p = context->pulseConfiguration[i];
            EventHandler::trigger(Event::PULSE_INT, p.index, p.value, p.interruptMode);
            p.isDirty = false;
        }
    }
}

} // namespace VMCBuddy
#endif // PULSEREADER_H
