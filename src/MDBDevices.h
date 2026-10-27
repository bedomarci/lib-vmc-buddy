#ifndef MDB_DEVICES_H
#define MDB_DEVICES_H

#include "MDBPeripheral.h"

namespace VMCBuddy {

/**
 * Pre-built MDB cashless reader peripheral (Level 2/3).
 *
 * Implements the common MDB cashless device commands:
 * RESET, SETUP, POLL, VEND, READER, EXPANSION.
 * Override the virtual handle* methods to implement device-specific logic.
 */
class MDBCashlessReader : public MDBPeripheral {
public:
    using MDBPeripheral::MDBPeripheral;
    const char* name() const override { return "MDB-Cashless"; }

protected:
    virtual uint8_t handlePoll()                     { return 0x00; }
    virtual void    handleReset()                    {}
    virtual void    handleSetup(const Packet& pkt)   {}
    virtual void    handleVend(const Packet& pkt)    {}
    virtual void    handleExpansion(const Packet& pkt) {}
    virtual void    handleReader(const Packet& pkt)  {}

    bool begin() override {
        if (!MDBPeripheral::begin()) return false;
        onPoll([this](){ return handlePoll(); });
        onReset([this](){ handleReset(); });
        onSetup([this](const Packet& p){ handleSetup(p); });
        onVend([this](const Packet& p){ handleVend(p); });
        onExpansion([this](const Packet& p){ handleExpansion(p); });
        return true;
    }
};

/**
 * Pre-built MDB bill validator peripheral.
 *
 * Handles the bill acceptor command set: RESET, SETUP, POLL,
 * VEND, EXPANSION.  Override handleBillAccept() for bill processing.
 */
class MDBBillValidator : public MDBPeripheral {
public:
    using MDBPeripheral::MDBPeripheral;
    const char* name() const override { return "MDB-BillVal"; }

protected:
    virtual uint8_t handlePoll()                { return 0x00; }
    virtual void    handleReset()               {}
    virtual void    handleBillAccept(const Packet& pkt) {}
    virtual void    handleExpansion(const Packet& pkt) {}

    bool begin() override {
        if (!MDBPeripheral::begin()) return false;
        onPoll([this](){ return handlePoll(); });
        onReset([this](){ handleReset(); });
        onVend([this](const Packet& p){ handleBillAccept(p); });
        onExpansion([this](const Packet& p){ handleExpansion(p); });
        return true;
    }
};

/**
 * Pre-built MDB coin changer peripheral.
 *
 * Implements the coin changer command set with separate coin-dispense
 * and tube-status reporting.  Override handleCoinDispense() for
 * coin output control.
 */
class MDBCoinChanger : public MDBPeripheral {
public:
    using MDBPeripheral::MDBPeripheral;
    const char* name() const override { return "MDB-CoinChg"; }

protected:
    virtual uint8_t handlePoll()                  { return 0x00; }
    virtual void    handleReset()                 {}
    virtual void    handleSetup(const Packet& pkt) {}
    virtual void    handleCoinDispense(const Packet& pkt) {}
    virtual void    handleExpansion(const Packet& pkt)   {}

    bool begin() override {
        if (!MDBPeripheral::begin()) return false;
        onPoll([this](){ return handlePoll(); });
        onReset([this](){ handleReset(); });
        onSetup([this](const Packet& p){ handleSetup(p); });
        onVend([this](const Packet& p){ handleCoinDispense(p); });
        onExpansion([this](const Packet& p){ handleExpansion(p); });
        return true;
    }
};

} // namespace VMCBuddy
#endif // MDB_DEVICES_H