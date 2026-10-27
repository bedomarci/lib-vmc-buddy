#ifndef ISERIAL_H
#define ISERIAL_H

#include <Arduino.h>
#include <IPeripheral.h>

namespace VMCBuddy {

/**
 * Common interface for every serial transport on the board:
 * RS-232, RS-485, MDB, and the UART command interpreter.
 *
 * Each subclass implements the physical-layer specifics
 * (hardware UART, bit-bang, direction control, 9-bit framing)
 * behind the same read/write/available/flush contract.
 *
 * Inherits IPeripheral → Task, so serial objects run on
 * the cooperative scheduler.
 */
class ISerial : public IPeripheral {
public:
    virtual ~ISerial() = default;

    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;

    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t* buf, size_t len) = 0;
    virtual void flush() = 0;
};

} // namespace VMCBuddy

#endif // ISERIAL_H