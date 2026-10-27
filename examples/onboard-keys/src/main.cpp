#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseReader.h>
#include <PulseIDC.h>
#include <PulseTerminal.h>
#include <EventHandler.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseReader pulseReader;
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

    connector1.setInterruptMode(1, FALLING);
    board.addPeripheral(&pulseReader);
    board.addPeripheral(&connector1);
    board.addPeripheral(&connector2);
    board.begin();
    connector2.sendPulses(1, 2, 100, 100); // local CH1 -> board output channel 4
    connector2.setInhibit(true);            // local INHIBIT -> board output channel 6
}

void loop() {
    scheduler.execute();
}
