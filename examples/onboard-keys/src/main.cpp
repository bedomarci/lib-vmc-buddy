#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseIDC.h>
#include <PulseTerminal.h>
#include <EventHandler.h>
#include <type_traits>

static_assert(std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulsePeripheral>::value,
              "PulsePeripheral must own the schedulable Task");
static_assert(std::is_convertible<VMCBuddy::PulsePeripheral*, VMCBuddy::IPeripheral*>::value,
              "PulsePeripheral must expose one unambiguous IPeripheral base");
static_assert(!std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulseReader>::value,
              "PulseReader must remain Task-free");
static_assert(!std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulseWriter>::value,
              "PulseWriter must remain Task-free");

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseIDC connector1(1, VMCBuddy::PulseIDC::Mode::Slave);
VMCBuddy::PulseIDC connector2(2, VMCBuddy::PulseIDC::Mode::Master);
VMCBuddy::PulseTerminal terminal(VMCBuddy::PulsePeripheral::Mode::Slave, 8);

void onPulse(int channel, int value, int mode) {
    Serial.printf("Pulse input %d: value=%d mode=%d\n", channel, value, mode);
}

void setup() {
    Serial.begin(115200);
    assert(connector1.pulseIndex(1) == 0 && connector1.pulseIndex(3) == 2);
    assert(connector2.pulseIndex(1) == 3 && connector2.pulseIndex(3) == 5);
    assert(connector2.gpio(1) == 33 && connector2.gpio(3) == 39);
    assert(terminal.pulseIndex(1) == 0 && terminal.pulseIndex(8) == 7);
    assert(terminal.inhibitChannel() == 8);
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::PULSE_INT, onPulse);

    const bool slaveDataInput = connector1.setInterruptMode(1, FALLING);
    const bool slaveInhibitOutput = !connector1.setInterruptMode(3, FALLING);
    const bool masterInhibitInput = connector2.setInterruptMode(3, FALLING);
    const bool masterDataOutput = !connector2.setInterruptMode(1, FALLING);
    assert(slaveDataInput && slaveInhibitOutput && masterInhibitInput && masterDataOutput);
    board.addPeripheral(&connector1);
    board.addPeripheral(&connector2);
    board.begin();
    assert(!terminal.begin()); // channel ownership prevents connector overlap
    connector2.sendPulses(1, 2, 100, 100); // local CH1 -> board output channel 4
    connector1.setInhibit(true);            // Slave drives local INHIBIT
}

void loop() {
    scheduler.execute();
}
