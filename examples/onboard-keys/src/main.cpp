// Program.ino
// License is available in LICENSE.md
// This program was created by <HUMAN>

#include "Arduino.h"   // go to this file to import libraries
#include "VMCBuddy.H"   // go to this file to import libraries
#include <TaskScheduler.h>


Scheduler *scheduler;
VMCBuddy *board;
void onChange(uint8_t event, int value);


void setup() {
    Serial.begin(115200);
    scheduler = new Scheduler();

    board = new VMCBuddy(scheduler);
    board->setOnChangeCallback(onChange);
    board->begin();
}

void onChange(uint8_t event, int value){
    Log.traceln("%d - %d", event, value);
}

void loop() {
    scheduler->execute();
}

