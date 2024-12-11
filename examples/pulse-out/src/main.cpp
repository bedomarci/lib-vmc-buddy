#include "Arduino.h"   // go to this file to import libraries
#include "VMCBuddy.H"   // go to this file to import libraries
#include <TaskScheduler.h>



Scheduler *scheduler;
VMCBuddy *board;
void flipPulseOut();
Task taskFlipPulseOut(300, TASK_FOREVER, &flipPulseOut);

bool state = false;

void setup() {
    Serial.begin(115200);
    scheduler = new Scheduler();

    board = new VMCBuddy(scheduler);
    scheduler->addTask(taskFlipPulseOut);

    taskFlipPulseOut.enable();
    board->pinModePulse(7, OUTPUT);
    board->pinModePulse(8, OUTPUT);
    board->begin();
}

void loop() {
    scheduler->execute();
}

void flipPulseOut() {
    board->digitalWritePulse(7,state );
    board->digitalWritePulse(8,!state );
    board->setLed(1, state);
    board->setLed(2, !state);

    state = !state;
}


