#include <Arduino.h>
#include <assert.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <PulseWriter.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::PulseWriter pulseWriter;
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    board.pinModePulse(6, OUTPUT); // zero-based channel 6 = physical pulse output 7
    board.addPeripheral(&pulseWriter);
    board.addPeripheral(&leds);
    board.begin();

    assert(!pulseWriter.sendPulses(NUM_PULSE, 1, 100, 100));
    assert(!leds.blink(NUM_LEDS, 1, 100, 100));
    const bool pulseStarted = pulseWriter.sendPulses(6, 3, 250, 250);
    const bool blinkStarted = leds.blink(0, 3, 200, 200);
    assert(pulseStarted); // 3 pulses, 250 ms high, 250 ms pause
    assert(blinkStarted);
}

void loop() {
    scheduler.execute();
}
