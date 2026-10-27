#ifndef SERIAL_PERIPHERAL_H
#define SERIAL_PERIPHERAL_H

#include <Arduino.h>
#include <ISerial.h>

namespace VMCBuddy {

/** Shared Stream adapter for serial peripherals that can use Arduino
 * HardwareSerial or a caller-provided Stream such as SoftwareSerial.
 * Injected streams must be configured by their owner before begin(). */
class SerialPeripheral : public ISerial {
public:
    void setSerial(Stream* serial);
    Stream* stream() const { return _serial; }

    bool begin() override;
    void end() override;
    int available() override;
    int read() override;
    int peek() override;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* data, size_t length) override;
    void flush() override;

protected:
    Stream* _serial = nullptr;
    bool _ownsSerial = false;

    /** Starts this peripheral's default hardware serial when no Stream
     * was injected. Set _serial and _ownsSerial before returning true. */
    virtual bool beginDefaultSerial() = 0;
    virtual void endDefaultSerial() = 0;
};

inline void SerialPeripheral::setSerial(Stream* serial) {
    if (_active) end();
    _serial = serial;
    _ownsSerial = false;
}

inline bool SerialPeripheral::begin() {
    if (_active) return true;
    if (!_serial && !beginDefaultSerial()) return false;
    if (!_serial) return false;
    _active = true;
    return true;
}

inline void SerialPeripheral::end() {
    if (_ownsSerial) {
        endDefaultSerial();
        _serial = nullptr;
        _ownsSerial = false;
    }
    _active = false;
}

inline int SerialPeripheral::available() { return _active ? _serial->available() : 0; }
inline int SerialPeripheral::read() { return _active ? _serial->read() : -1; }
inline int SerialPeripheral::peek() { return _active ? _serial->peek() : -1; }
inline size_t SerialPeripheral::write(uint8_t c) { return _active ? _serial->write(c) : 0; }
inline size_t SerialPeripheral::write(const uint8_t* data, size_t length) {
    return _active ? _serial->write(data, length) : 0;
}
inline void SerialPeripheral::flush() { if (_active) _serial->flush(); }

} // namespace VMCBuddy
#endif // SERIAL_PERIPHERAL_H
