#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseIDC.h>
#include <PulseTerminal.h>
#include <EventHandler.h>
#include <type_traits>

static_assert(std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulsePeripheral>::value,
              "PulsePeripheral must be IPeripheral");
static_assert(std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulseChannel>::value,
              "PulseChannel must be IPeripheral");
static_assert(std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulseIDC>::value,
              "PulseIDC must be IPeripheral");
static_assert(std::is_base_of<VMCBuddy::IPeripheral, VMCBuddy::PulseTerminal>::value,
              "PulseTerminal must be IPeripheral");

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseIDC connector1(1, VMCBuddy::PulseIDC::Mode::Slave);
VMCBuddy::PulseIDC connector2(2, VMCBuddy::PulseIDC::Mode::Master);

VMCBuddy::PulseTerminal::TerminalChannel termChannels[] = {
    {1, INPUT},
    {5, OUTPUT},
    {8, INPUT}
};
VMCBuddy::PulseTerminal terminal(termChannels, 3, 3);

void onPulse(int channel, int value, int mode) {
    Serial.printf("Pulse input %d: value=%d mode=%d\n", channel, value, mode);
}

void onChannel(int channel, int param, int mode) {
    Serial.printf("Channel %d: param=%d mode=%d\n", channel, param, mode);
}

void setup() {
    Serial.begin(115200);
    assert(connector1.pulseIndex(1) == 0 && connector1.pulseIndex(3) == 2);
    assert(connector2.pulseIndex(1) == 3 && connector2.pulseIndex(3) == 5);
    assert(connector2.gpio(1) == 33 && connector2.gpio(3) == 39);
    assert(terminal.pulseIndex(1) == 0 && terminal.pulseIndex(3) == 7);
    assert(terminal.inhibitChannel() == 3);
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::PULSE_INT, onPulse);
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::CHANNEL_START, onChannel);
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::CHANNEL_STOP, onChannel);
    VMCBuddy::EventHandler::subscribe(VMCBuddy::Event::CHANNEL_TICK, onChannel);

    const bool slaveDataInput = connector1.setInterruptMode(1, FALLING);
    const bool slaveInhibitOutput = !connector1.setInterruptMode(3, FALLING);
    const bool masterInhibitInput = connector2.setInterruptMode(3, FALLING);
    const bool masterDataOutput = !connector2.setInterruptMode(1, FALLING);
    assert(slaveDataInput && slaveInhibitOutput && masterInhibitInput && masterDataOutput);
    board.addPeripheral(&connector1);
    board.addPeripheral(&connector2);
    board.begin();
    assert(!terminal.begin()); // channel ownership prevents connector overlap
    connector2.sendPulses(1, 2, 100, 100);
    connector1.setInhibit(true);
}

void loop() {
    scheduler.execute();
}