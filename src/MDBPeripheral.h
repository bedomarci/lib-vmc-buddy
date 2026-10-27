#ifndef MDB_PERIPHERAL_H
#define MDB_PERIPHERAL_H

#include <Arduino.h>
#include <functional>
#include "MDB.h"

namespace VMCBuddy {

/**
 * MDB peripheral state machine (L3).
 *
 * Inherits MDB (bit-bang 9-bit driver) and polls the MDB bus every
 * 1 ms. Handles RESET, POLL, SETUP, VEND, EXPANSION commands and
 * dispatches to registered callbacks.
 *
 * CommandInterpreter can be attached to the same peripheral for
 * runtime configuration over RS-232 or MDB.
 *
 * Usage:
 *   MDBPeripheral periph(0x10);
 *   periph.onPoll([](){ return 0x01; });
 *   periph.onVend([](const MDB::Packet& p){ ... });
 *   board.addPeripheral(&periph);
 */
class MDBPeripheral : public MDB {
public:
    explicit MDBPeripheral(uint8_t address,
                           Print* logOut = nullptr);

    bool begin() override;  // starts bus + enables task
    void end() override;    // stops task + releases bus
    const char* name() const override { return "MDB-Peripheral"; }

    using PollHandler      = std::function<uint8_t()>;
    using ResetHandler     = std::function<void()>;
    using SetupHandler     = std::function<void(const MDB::Packet&)>;
    using VendHandler      = std::function<void(const MDB::Packet&)>;
    using ExpansionHandler = std::function<void(const MDB::Packet&)>;

    void onPoll(PollHandler h)           { _onPoll = h; }
    void onReset(ResetHandler h)         { _onReset = h; }
    void onSetup(SetupHandler h)         { _onSetup = h; }
    void onVend(VendHandler h)           { _onVend = h; }
    void onExpansion(ExpansionHandler h) { _onExpansion = h; }

private:
    Print* _serial;
    bool _enabled   = false;
    bool _justReset = true;

    PollHandler      _onPoll;
    ResetHandler     _onReset;
    SetupHandler     _onSetup;
    VendHandler      _onVend;
    ExpansionHandler _onExpansion;

    bool tick();
    void dispatch(const Packet& packet);
};

inline MDBPeripheral::MDBPeripheral(uint8_t address, Print* logOut)
    : MDB(address), _serial(logOut) {
    setInterval(1);
    setIterations(TASK_FOREVER);
    setCallback([this](){ if(_enabled) tick(); });
}

inline bool MDBPeripheral::begin() {
    if (!MDB::begin()) { if (_serial) _serial->println(F("MDB: begin failed")); return false; }
    _enabled   = true;
    _justReset = true;
    Task::enable();
    _active = true;
    return true;
}

inline void MDBPeripheral::end() {
    _enabled = false;
    Task::disable();
    MDB::end();
    _active = false;
}

inline bool MDBPeripheral::tick() {
    Packet* pkt = readPacket(5);
    if (!pkt || !pkt->valid) return true;
    if (pkt->addr != address()) return true;
    dispatch(*pkt);
    return true;
}

inline void MDBPeripheral::dispatch(const Packet& packet) {
    if (_justReset) {
        if (packet.cmd == 0x08) { _justReset = false; if (_onReset) _onReset(); sendPacket(0x00); }
        return;
    }
    switch (packet.cmd) {
    case 0x08: _justReset = true; if (_onReset) _onReset(); sendPacket(0x00); break;
    case 0x0B: sendPacket(_onPoll ? _onPoll() : 0x00); break;
    case 0x0A: if (_onVend) _onVend(packet); sendPacket(0x00); break;
    case 0x11: if (_onExpansion) _onExpansion(packet); sendPacket(0x00); break;
    case 0x0F: if (_onSetup) _onSetup(packet); sendPacket(0x00); break;
    default:   sendPacket(0x00); break;
    }
}

} // namespace VMCBuddy
#endif // MDB_PERIPHERAL_H
