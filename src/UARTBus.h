#ifndef UART_BUS_H
#define UART_BUS_H

#include <Arduino.h>
#include <macros.h>

namespace VMCBuddy {

/** Tracks shared UART/GPIO routing claims so mutually-exclusive board
 * interfaces cannot start together. MDB also claims UART2's GPIO route
 * because its bit-bang pins share the same jumper-selected nets. */
class UARTBus {
public:
    enum Owner { NONE = 0, MDB = 1, RS485 = 2, RS232_CH1 = 3, RS232_CH2 = 4, CMD = 5 };

    static bool acquire(uint8_t uartNum, Owner owner);
    static void release(uint8_t uartNum);
    static Owner owner(uint8_t uartNum);
};

// ponytail: static array in function avoids C++17 inline variable requirement
inline UARTBus::Owner& _ownerRef(uint8_t uartNum) {
    static UARTBus::Owner owners[3] = {UARTBus::NONE, UARTBus::NONE, UARTBus::NONE};
    return owners[uartNum];
}

inline bool UARTBus::acquire(uint8_t uartNum, Owner owner) {
    if (uartNum > 2) return false;
    if (_ownerRef(uartNum) != NONE) return false;
    _ownerRef(uartNum) = owner;
    return true;
}

inline void UARTBus::release(uint8_t uartNum) {
    if (uartNum <= 2) _ownerRef(uartNum) = NONE;
}

inline UARTBus::Owner UARTBus::owner(uint8_t uartNum) {
    return (uartNum <= 2) ? _ownerRef(uartNum) : NONE;
}

/** Runtime sanity check for GPIO uniqueness and shift-register bit ranges. */
inline bool validateBoardConfig() {
    const auto& cfg = Context::getInstance()->boardConfig;
    const uint8_t assigned[] = {
        cfg.srLatch, cfg.srData, cfg.srClock, cfg.btnAdc,
        cfg.pulseInGpio[0], cfg.pulseInGpio[1], cfg.pulseInGpio[2], cfg.pulseInGpio[3],
        cfg.pulseInGpio[4], cfg.pulseInGpio[5], cfg.pulseInGpio[6], cfg.pulseInGpio[7],
        cfg.rs485TxEn, cfg.rs232Ch1Tx, cfg.rs232Ch1Rx, cfg.uart2Tx, cfg.uart2Rx,
        cfg.wiegandInD0, cfg.wiegandInD1, cfg.wiegandOutD0, cfg.wiegandOutD1,
    };
    for (size_t i = 0; i < sizeof(assigned); ++i) {
        for (size_t j = i + 1; j < sizeof(assigned); ++j) {
            if (assigned[i] == assigned[j]) {
                return false;
            }
        }
    }

    uint16_t shiftRegisterBits = 0;
    for (uint8_t i = 0; i < cfg.numPulses; ++i) {
        const uint8_t bit = cfg.pulseOutSrBit[i];
        if (bit >= cfg.srBitCount || (shiftRegisterBits & (1U << bit))) return false;
        shiftRegisterBits |= 1U << bit;
    }
    for (uint8_t i = 0; i < cfg.numLeds; ++i) {
        const uint8_t bit = cfg.ledSrBit[i];
        if (bit >= cfg.srBitCount || (shiftRegisterBits & (1U << bit))) return false;
        shiftRegisterBits |= 1U << bit;
    }
    return true;
}

} // namespace VMCBuddy

#endif // UART_BUS_H
