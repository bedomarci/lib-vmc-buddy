#ifndef RS485_H
#define RS485_H

#include <Arduino.h>
#include <SerialPeripheral.h>
#include <Context.h>
#include <UARTBus.h>

namespace VMCBuddy {

/** Half-duplex RS-485 wrapper. Defaults to HardwareSerial2 but accepts
 * a caller-configured HardwareSerial/SoftwareSerial through
 * setSerial(Stream*). Write calls control the board DE/RE pin. */
class RS485 : public SerialPeripheral {
public:
    RS485();

    bool begin() override;
    bool begin(unsigned long baud, uint32_t config = SERIAL_8N1);
    void end() override;
    const char* name() const override { return "RS485"; }

    size_t write(uint8_t c) override;
    size_t write(const uint8_t* data, size_t length) override;

protected:
    bool beginDefaultSerial() override;
    void endDefaultSerial() override;

private:
    uint8_t _dePin;
    unsigned long _baud = 9600;
    uint32_t _config = SERIAL_8N1;
    bool _claimed = false;
};

inline RS485::RS485() : SerialPeripheral(), _dePin(Context::getInstance()->boardConfig.rs485TxEn) {
    pinMode(_dePin, OUTPUT);
    digitalWrite(_dePin, LOW);
}

inline bool RS485::begin() {
    if (_active) return true;
    if (!UARTBus::acquire(2, UARTBus::RS485)) return false;
    _claimed = true;
    if (!SerialPeripheral::begin()) {
        UARTBus::release(2);
        _claimed = false;
        return false;
    }
    return true;
}

inline bool RS485::begin(unsigned long baud, uint32_t config) {
    _baud = baud;
    _config = config;
    return begin();
}

inline bool RS485::beginDefaultSerial() {
    const auto& cfg = Context::getInstance()->boardConfig;
    _serial = &Serial2;
    _ownsSerial = true;
    Serial2.begin(_baud, _config, cfg.uart2Rx, cfg.uart2Tx);
    return true;
}

inline void RS485::endDefaultSerial() { Serial2.end(); }

inline void RS485::end() {
    if (!_active) return;
    digitalWrite(_dePin, LOW);
    SerialPeripheral::end();
    if (_claimed) UARTBus::release(2);
    _claimed = false;
}

inline size_t RS485::write(uint8_t c) {
    if (!_active) return 0;
    digitalWrite(_dePin, HIGH);
    delayMicroseconds(20);
    size_t written = _serial->write(c);
    _serial->flush();
    digitalWrite(_dePin, LOW);
    return written;
}

inline size_t RS485::write(const uint8_t* data, size_t length) {
    if (!_active) return 0;
    digitalWrite(_dePin, HIGH);
    delayMicroseconds(20);
    size_t written = _serial->write(data, length);
    _serial->flush();
    digitalWrite(_dePin, LOW);
    return written;
}

} // namespace VMCBuddy
#endif // RS485_H
