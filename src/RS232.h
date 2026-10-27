#ifndef RS232_H
#define RS232_H

#include <Arduino.h>
#include <ISerial.h>
#include <macros.h>
#include <UARTBus.h>

namespace VMCBuddy {

/**
 * HardwareSerial wrapper for one RS-232 channel.
 *
 * Channel 1 uses UART1 with the board's dedicated TX/RX GPIOs.
 * Channel 2 uses UART2 — shared with RS-485 and MDB via the
 * 2×15 through-hole jumper field (only one path active at a time).
 *
 * Usage:
 *   RS232 ch1(1);
 *   ch1.begin(9600);
 *   ch1.write("hello\n");
 */
class RS232 : public ISerial {
public:
    explicit RS232(uint8_t channel);
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

    const char* name() const override { return _channel == 1 ? "RS232-ch1" : "RS232-ch2"; }
    HardwareSerial* stream() { return _serial; }

private:
    HardwareSerial* _serial;
    uint8_t _tx, _rx;
    uint8_t _channel;
    uint8_t _uartNum;
};

inline RS232::RS232(uint8_t channel) : ISerial() {
    const auto& cfg = boardCfg();
    _channel = channel;
    if (channel == 1) {
        _uartNum = 1;
        _tx = cfg.rs232Ch1Tx;
        _rx = cfg.rs232Ch1Rx;
        _serial = &Serial1;
    } else {
        _uartNum = 2;
        _tx = cfg.rs232Ch2Tx;
        _rx = cfg.rs232Ch2Rx;
        _serial = &Serial2;
    }
}

inline bool RS232::begin(unsigned long baud, uint32_t config) {
    if (_active) return true;
    const auto owner = _channel == 1 ? UARTBus::RS232_CH1 : UARTBus::RS232_CH2;
    if (!UARTBus::acquire(_uartNum, owner)) return false;
    _serial->begin(baud, config, _rx, _tx);
    _active = true;
    return true;
}

inline void RS232::end() {
    if (!_active) return;
    _serial->end();
    UARTBus::release(_uartNum);
    _active = false;
}
inline int RS232::available() { return _active ? _serial->available() : 0; }
inline int RS232::read()      { return _active ? _serial->read() : -1; }
inline int RS232::peek()      { return _active ? _serial->peek() : -1; }
inline size_t RS232::write(uint8_t c) { return _active ? _serial->write(c) : 0; }
inline size_t RS232::write(const uint8_t* buf, size_t len) { return _active ? _serial->write(buf, len) : 0; }
inline void RS232::flush() { if (_active) _serial->flush(); }

} // namespace VMCBuddy
#endif // RS232_H
