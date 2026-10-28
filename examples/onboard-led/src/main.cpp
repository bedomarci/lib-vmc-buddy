#include <Arduino.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <Led.h>
#include <Relay.h>
#include <assert.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::Led leds;
VMCBuddy::Relay relays;

void setup() {
    Serial.begin(115200);
    board.addPeripheral(&leds);
    board.addPeripheral(&relays);
    board.begin();

    assert(relays.set(1, false));
    assert(relays.set(2, false));
    assert(!relays.set(0, true));
    assert(!relays.set(3, true));

    // LED index is zero-based: 0..5. Blink 5 times, 300 ms on/off.
    leds.blink(0, 5, 300, 300);
}

void loop() {
    scheduler.execute();
}
