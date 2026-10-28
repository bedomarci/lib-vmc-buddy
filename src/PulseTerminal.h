#ifndef PULSE_TERMINAL_H
#define PULSE_TERMINAL_H

#include <PulsePeripheral.h>

namespace VMCBuddy {

class PulseTerminal : public PulsePeripheral {
public:
    struct TerminalChannel {
        uint8_t boardIndex; // 1..NUM_PULSE
        uint8_t direction;  // INPUT or OUTPUT
    };

    explicit PulseTerminal(const TerminalChannel* channels, uint8_t count,
                           uint8_t inhibitLocalIndex = 0);

    PulseTerminal();
    PulseTerminal& addChannel(uint8_t boardIndex, uint8_t direction);
    PulseTerminal& setInhibitIndex(uint8_t localIndex);

    const char* name() const override { return "PulseTerminal"; }

protected:
    uint8_t channelCount() const override { return _channelCount; }
    int mapChannel(uint8_t localChannel) const override;

    using PulsePeripheral::isInputChannel;
    bool isInputChannel(uint8_t localChannel) const;

private:
    TerminalChannel _channelList[8] = {};
    uint8_t _channelCount = 0;
};

inline PulseTerminal::PulseTerminal(const TerminalChannel* channels, uint8_t count,
                                     uint8_t inhibitLocalIndex)
    : PulsePeripheral(Mode::Master, inhibitLocalIndex), _channelCount(count) {
    for (uint8_t i = 0; i < count && i < 8; ++i)
        _channelList[i] = channels[i];
}

inline PulseTerminal::PulseTerminal() : PulsePeripheral(Mode::Master, 0) {}

inline PulseTerminal& PulseTerminal::addChannel(uint8_t boardIndex, uint8_t direction) {
    if (_active || _channelCount >= 8) return *this;
    _channelList[_channelCount++] = {boardIndex, direction};
    return *this;
}

inline PulseTerminal& PulseTerminal::setInhibitIndex(uint8_t localIndex) {
    if (_active || localIndex > _channelCount) return *this;
    _inhibitChannel = localIndex; // ponytail: direct member access from parent
    return *this;
}

inline int PulseTerminal::mapChannel(uint8_t localChannel) const {
    if (localChannel < 1 || localChannel > _channelCount) return -1;
    return _channelList[localChannel - 1].boardIndex - 1;
}

inline bool PulseTerminal::isInputChannel(uint8_t localChannel) const {
    if (localChannel < 1 || localChannel > _channelCount) return false;
    return _channelList[localChannel - 1].direction == INPUT;
}

} // namespace VMCBuddy
#endif // PULSE_TERMINAL_H