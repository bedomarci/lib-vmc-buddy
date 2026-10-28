#ifndef PULSE_TERMINAL_H
#define PULSE_TERMINAL_H

#include <PulsePeripheral.h>

namespace VMCBuddy {

/** Direction-configurable adapter for the 10-pin pulse terminal.
 * Terminal pins 1–8 map directly to board pulse channels 1–8.
 * Any one of those local channels can be selected as INHIBIT. */
class PulseTerminal : public PulsePeripheral {
public:
    using Mode = PulsePeripheral::Mode;

    explicit PulseTerminal(Mode mode = Mode::Slave, uint8_t inhibitChannel = 0);
    const char* name() const override { return "PulseTerminal"; }

protected:
    uint8_t channelCount() const override {
        return Context::getInstance()->boardConfig.numPulses;
    }
    int mapChannel(uint8_t localChannel) const override;
};

inline PulseTerminal::PulseTerminal(Mode mode, uint8_t inhibitChannel)
    : PulsePeripheral(mode, inhibitChannel) {}

inline int PulseTerminal::mapChannel(uint8_t localChannel) const {
    return localChannel >= 1 && localChannel <= channelCount() ? localChannel - 1 : -1;
}

} // namespace VMCBuddy
#endif // PULSE_TERMINAL_H
