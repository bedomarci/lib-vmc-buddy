#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseTerminal.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseTerminal pulsePort(VMCBuddy::PulseTerminal::Mode::Master);
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    board.addPeripheral(&pulsePort);
    board.addPeripheral(&leds);
    board.begin();

    assert(!pulsePort.sendPulses(0, 1, 100, 100));
    assert(!pulsePort.sendPulses(NUM_PULSE + 1, 1, 100, 100));
    assert(!leds.blink(NUM_LEDS, 1, 100, 100));
    const bool pulseStarted = pulsePort.sendPulses(7, 3, 250, 250);
    const bool blinkStarted = leds.blink(0, 3, 200, 200);
    assert(pulseStarted); // 3 pulses, 250 ms high, 250 ms pause
    assert(blinkStarted);
}

void loop() {
    scheduler.execute();
}
