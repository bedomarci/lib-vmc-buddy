#include <Arduino.h>
#include <VMCBuddy.h>
#include <TaskScheduler.h>
#include <Led.h>

Scheduler scheduler;
VMCBuddy::VMCBuddy board(&scheduler);
VMCBuddy::Led leds;

void setup() {
    Serial.begin(115200);
    board.addPeripheral(&leds);
    board.begin();

    // LED index is zero-based: 0..5. Blink 5 times, 300 ms on/off.
    leds.blink(0, 5, 300, 300);
}

void loop() {
    scheduler.execute();
}
