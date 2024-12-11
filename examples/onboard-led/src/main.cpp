// Program.ino
// License is available in LICENSE.md
// This program was created by <HUMAN>

#include "Arduino.h"   // go to this file to import libraries
#include "VMCBuddy.H"   // go to this file to import libraries
#include <TaskScheduler.h>
//#include <TaskSchedulerDeclarations.h>


Scheduler *scheduler;
VMCBuddy *board;
void testLed();

Task taskTestLed(200, TASK_FOREVER, &testLed);

void setup() {
    Serial.begin(115200);
    scheduler = new Scheduler();

    board = new VMCBuddy(scheduler);
    board->begin();

    scheduler->addTask(taskTestLed);
    taskTestLed.enable();
}

void testLed() {
    int ind = taskTestLed.getRunCounter()%6;
    int val = taskTestLed.getRunCounter()%12;
    val = (val < 6) ? HIGH : LOW;
    board->setLed(ind, val);

}

void loop() {
    scheduler->execute();
}

