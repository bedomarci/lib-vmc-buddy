#ifndef PULSEREADER_H
#define PULSEREADER_H

#include <EventHandler.h>
#include <Context.h>

namespace VMCBuddy {

/** Task-free pulse input behavior. Owning peripherals check only their mapped inputs. */
class PulseReader {
public:
    static bool setInterruptMode(uint8_t index, uint8_t mode,
                                 uint16_t debounce = DEFAULT_PULSE_DEBOUNCE);

protected:
    PulseReader() = default;
    ~PulseReader() = default;

    static bool interruptEnabled(uint8_t index);
    static void attachInterruptFor(uint8_t index);
    static void detachInterruptFor(uint8_t index);
    static void checkDirty(uint8_t index);

private:
    static void IRAM_ATTR handleInterruptArg(void* arg);
    static void IRAM_ATTR handleInterrupts(uint8_t pulseIndex);
};

inline bool PulseReader::setInterruptMode(uint8_t index, uint8_t mode, uint16_t debounce) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return false;
    auto& pulse = context->pulseConfiguration[index];
    if (pulse.pinMode != INPUT) {
        pulse.interruptMode = DETACH_ISR;
        return false;
    }
    if (mode != DETACH_ISR && mode != CHANGE && mode != RISING && mode != FALLING)
        return false;
    pulse.interruptMode = mode;
    pulse.debounce = debounce;
    return true;
}

inline bool PulseReader::interruptEnabled(uint8_t index) {
    auto* context = Context::getInstance();
    return index < context->boardConfig.numPulses &&
           context->pulseConfiguration[index].pinMode == INPUT &&
           context->pulseConfiguration[index].interruptMode != DETACH_ISR;
}

inline void PulseReader::attachInterruptFor(uint8_t index) {
    if (!interruptEnabled(index)) return;
    auto* context = Context::getInstance();
    auto& pulse = context->pulseConfiguration[index];
    pulse.isDirty = false;
    attachInterruptArg(context->boardConfig.pulseInGpio[index], PulseReader::handleInterruptArg,
                       reinterpret_cast<void*>(static_cast<uintptr_t>(index + 1)),
                       pulse.interruptMode);
}

inline void PulseReader::detachInterruptFor(uint8_t index) {
    auto* context = Context::getInstance();
    if (index < context->boardConfig.numPulses)
        detachInterrupt(context->boardConfig.pulseInGpio[index]);
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

inline void PulseReader::checkDirty(uint8_t index) {
    auto* context = Context::getInstance();
    if (index >= context->boardConfig.numPulses) return;
    auto& pulse = context->pulseConfiguration[index];
    if (!pulse.isDirty || pulse.pinMode != INPUT) return;
    EventHandler::trigger(Event::PULSE_INT, pulse.index, pulse.value, pulse.interruptMode);
    pulse.isDirty = false;
}

} // namespace VMCBuddy
#endif // PULSEREADER_H
