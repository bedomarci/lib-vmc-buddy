#ifndef RS485_H
#define RS485_H

#include <Arduino.h>
#include <ISerial.h>
#include <macros.h>
#include <UARTBus.h>

namespace VMCBuddy {

/**
 * Half-duplex RS-485 transceiver wrapper.
 *
 * Uses HardwareSerial2 with automatic direction-control toggling
 * (DE/RE pin HIGH during TX, LOW for RX). Shares UART2 with RS-232
 * channel 2 and MDB — only one path can be active via the 2×15
 * through-hole jumper field.
 *
 * Usage:
 *   RS485 bus;
 *   bus.begin(9600);
 *   bus.write(data, len);  // auto toggles DE/RE
 */
class RS485 : public ISerial {
public:
    RS485();
    /** IPeripheral contract: begin with default 9600 8N1. */
    bool begin() override { return begin(9600, SERIAL_8N1); }
    bool begin(unsigned long baud, uint32_t config = SERIAL_8N1);
    void end() override;

    int available() override;
    int read() override;
    int peek() override;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buf, size_t len) override;
    void flush() override;

    const char* name() const override { return "RS485"; }
    HardwareSerial* stream() { return _serial; }

private:
    HardwareSerial* _serial;
    uint8_t _dePin;
};

inline RS485::RS485() : ISerial() {
    const auto& cfg = boardCfg();
    _dePin  = cfg.rs485TxEn;
    _serial = &Serial2;
    pinMode(_dePin, OUTPUT);
    digitalWrite(_dePin, LOW);
}

inline bool RS485::begin(unsigned long baud, uint32_t config) {
    if (_active) return true;
    if (!UARTBus::acquire(2, UARTBus::RS485)) return false;
    const auto& cfg = boardCfg();
    _serial->begin(baud, config, cfg.uart2Rx, cfg.uart2Tx);
    _active = true;
    return true;
}

inline void RS485::end() {
    if (!_active) return;
    _serial->end();
    UARTBus::release(2);
    _active = false;
}
inline int RS485::available() { return _active ? _serial->available() : 0; }
inline int RS485::read()      { return _active ? _serial->read() : -1; }
inline int RS485::peek()      { return _active ? _serial->peek() : -1; }

inline size_t RS485::write(uint8_t c) {
    if (!_active) return 0;
    digitalWrite(_dePin, HIGH);
    delayMicroseconds(20);
    size_t w = _serial->write(c);
    _serial->flush();
    digitalWrite(_dePin, LOW);
    return w;
}

inline size_t RS485::write(const uint8_t* buf, size_t len) {
    if (!_active) return 0;
    digitalWrite(_dePin, HIGH);
    delayMicroseconds(20);
    size_t w = _serial->write(buf, len);
    _serial->flush();
    digitalWrite(_dePin, LOW);
    return w;
}

inline void RS485::flush() { if (_active) _serial->flush(); }

} // namespace VMCBuddy
#endif // RS485_H
