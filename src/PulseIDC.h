#ifndef PULSE_IDC_H
#define PULSE_IDC_H

#include <PulsePeripheral.h>

namespace VMCBuddy {

/** Direction-configurable adapter for one three-signal pulse IDC.
 * Local CH1, CH2, INHIBIT contacts are IDC pins 8, 6, 4. Connector 1
 * maps to board pulse channels 1–3; connector 2 maps to channels 4–6. */
class PulseIDC : public PulsePeripheral {
public:
    using Mode = PulsePeripheral::Mode;

    explicit PulseIDC(uint8_t connectorNumber, Mode mode = Mode::Slave);

    const char* name() const override;
    int idcPin(uint8_t localChannel) const;
    bool setInhibitChannel(uint8_t localChannel) override {
        return localChannel == 0 || localChannel == 3
            ? PulsePeripheral::setInhibitChannel(localChannel)
            : false;
    }

protected:
    uint8_t channelCount() const override { return 3; }
    int mapChannel(uint8_t localChannel) const override;

private:
    uint8_t _connectorNumber;
};

inline PulseIDC::PulseIDC(uint8_t connectorNumber, Mode mode)
    : PulsePeripheral(mode, 3), _connectorNumber(connectorNumber) {}

inline const char* PulseIDC::name() const {
    return _connectorNumber == 1 ? "PulseIDC1" :
           _connectorNumber == 2 ? "PulseIDC2" : "PulseIDC-invalid";
}

inline int PulseIDC::mapChannel(uint8_t localChannel) const {
    if ((_connectorNumber != 1 && _connectorNumber != 2) ||
        localChannel < 1 || localChannel > channelCount()) return -1;
    return (_connectorNumber - 1) * channelCount() + localChannel - 1;
}

inline int PulseIDC::idcPin(uint8_t localChannel) const {
    return mapChannel(localChannel) >= 0 ? 10 - 2 * localChannel : -1;
}

} // namespace VMCBuddy
#endif // PULSE_IDC_H
