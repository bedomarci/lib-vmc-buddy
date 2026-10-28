#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseTerminal.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseTerminal::TerminalChannel chs[] = {
    {1, OUTPUT}, {2, OUTPUT}, {3, OUTPUT}, {4, OUTPUT},
    {5, OUTPUT}, {6, OUTPUT}, {7, OUTPUT}, {8, OUTPUT}
};
VMCBuddy::PulseTerminal pulsePort(chs, 8);
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    board.addPeripheral(&pulsePort);
    board.addPeripheral(&leds);
    board.begin();

    assert(!pulsePort.sendPulses(0, 1, 100, 100));
    assert(!pulsePort.sendPulses(9, 1, 100, 100));
    assert(!leds.blink(NUM_LEDS, 1, 100, 100));
    const bool pulseStarted = pulsePort.sendPulses(7, 3, 250, 250);
    const bool blinkStarted = leds.blink(0, 3, 200, 200);
    assert(pulseStarted);
    assert(blinkStarted);
}

void loop() {
    scheduler.execute();
}