#ifndef RS232_H
#define RS232_H

#include <Arduino.h>
#include <SerialPeripheral.h>
#include <Context.h>
#include <UARTBus.h>

namespace VMCBuddy {

/** RS-232 channel wrapper. Uses board HardwareSerial by default, or
 * accepts a caller-configured HardwareSerial/SoftwareSerial via
 * setSerial(Stream*). Injected streams remain caller-owned. */
class RS232 : public SerialPeripheral {
public:
    explicit RS232(uint8_t channel);

    bool begin() override;
    bool begin(unsigned long baud, uint32_t config = SERIAL_8N1);
    void end() override;
    const char* name() const override;

protected:
    bool beginDefaultSerial() override;
    void endDefaultSerial() override;

private:
    uint8_t _channel;
    uint8_t _uartNum;
    uint8_t _tx = 0;
    uint8_t _rx = 0;
    unsigned long _baud = 9600;
    uint32_t _config = SERIAL_8N1;
    bool _validChannel = false;
    bool _claimed = false;
};

inline RS232::RS232(uint8_t channel) : SerialPeripheral(), _channel(channel) {
    const auto& cfg = Context::getInstance()->boardConfig;
    _validChannel = channel == 1 || channel == 2;
    if (!_validChannel) return;
    if (channel == 1) {
        _uartNum = 1;
        _tx = cfg.rs232Ch1Tx;
        _rx = cfg.rs232Ch1Rx;
    } else {
        _uartNum = 2;
        _tx = cfg.uart2Tx;
        _rx = cfg.uart2Rx;
    }
}

inline const char* RS232::name() const {
    return _channel == 1 ? "RS232-ch1" : _channel == 2 ? "RS232-ch2" : "RS232-invalid";
}

inline bool RS232::begin() {
    if (!_validChannel) return false;
    if (_active) return true;
    const auto owner = _channel == 1 ? UARTBus::RS232_CH1 : UARTBus::RS232_CH2;
    if (!UARTBus::acquire(_uartNum, owner)) return false;
    _claimed = true;
    if (!SerialPeripheral::begin()) {
        UARTBus::release(_uartNum);
        _claimed = false;
        return false;
    }
    return true;
}

inline bool RS232::begin(unsigned long baud, uint32_t config) {
    _baud = baud;
    _config = config;
    return begin();
}

inline bool RS232::beginDefaultSerial() {
    auto* hw = _channel == 1 ? &Serial1 : &Serial2;
    _serial = hw;
    _ownsSerial = true;
    hw->begin(_baud, _config, _rx, _tx);
    return true;
}

inline void RS232::endDefaultSerial() {
    static_cast<HardwareSerial*>(_serial)->end();
}

inline void RS232::end() {
    if (!_active) return;
    SerialPeripheral::end();
    if (_claimed) UARTBus::release(_uartNum);
    _claimed = false;
}

} // namespace VMCBuddy
#endif // RS232_H
