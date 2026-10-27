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
    Context::getInstance()->pulseConfiguration[index].interruptMode = mode;
    Context::getInstance()->pulseConfiguration[index].debounce = debounce;
}

inline bool PulseReader::registerInterrupts() {
    if (Context::getInstance()->pulseConfiguration[0].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[0].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(0);},
                       Context::getInstance()->pulseConfiguration[0].interruptMode);
    if (Context::getInstance()->pulseConfiguration[1].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[1].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(1);},
                       Context::getInstance()->pulseConfiguration[1].interruptMode);
    if (Context::getInstance()->pulseConfiguration[2].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[2].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(2);},
                       Context::getInstance()->pulseConfiguration[2].interruptMode);
    if (Context::getInstance()->pulseConfiguration[3].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[3].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(3);},
                       Context::getInstance()->pulseConfiguration[3].interruptMode);
    if (Context::getInstance()->pulseConfiguration[4].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[4].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(4);},
                       Context::getInstance()->pulseConfiguration[4].interruptMode);
    if (Context::getInstance()->pulseConfiguration[5].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[5].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(5);},
                       Context::getInstance()->pulseConfiguration[5].interruptMode);
    if (Context::getInstance()->pulseConfiguration[6].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[6].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(6);},
                       Context::getInstance()->pulseConfiguration[6].interruptMode);
    if (Context::getInstance()->pulseConfiguration[7].interruptMode)
        attachInterrupt(Context::getInstance()->pulseConfiguration[7].pin,
                       []()IRAM_ATTR{PulseReader::handleInterrupts(7);},
                       Context::getInstance()->pulseConfiguration[7].interruptMode);
    bool hasInterrupts = false;
    for (const auto& p : Context::getInstance()->pulseConfiguration)
        hasInterrupts |= (p.interruptMode != DETACH_ISR);
    return hasInterrupts;
}

inline void IRAM_ATTR PulseReader::handleInterrupts(const uint8_t pulseIndex) {
    auto& pulse = Context::getInstance()->pulseConfiguration[pulseIndex];
    if (pulse.pinMode != INPUT) return;
    const unsigned long now = millis();
    if (now - pulse.lastInterrupt > pulse.debounce) {
        pulse.value = digitalRead(pulse.pin);
        pulse.isDirty = true;
        pulse.lastInterrupt = now;
    }
}

inline void PulseReader::checkDirty() {
    for (int i = 0; i < LEN_PIN_INTERRUPTS; i++) {
        if (Context::getInstance()->pulseConfiguration[i].isDirty) {
            auto& p = Context::getInstance()->pulseConfiguration[i];
            EventHandler::trigger(Event::PULSE_INT, p.index, p.value, p.interruptMode);
            p.isDirty = false;
        }
    }
}

} // namespace VMCBuddy
#endif // PULSEREADER_H