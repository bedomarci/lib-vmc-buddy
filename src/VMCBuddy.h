#ifndef VMC_BUDDY_H
#define VMC_BUDDY_H

#define _TASK_STD_FUNCTION

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <ArduinoLog.h>
#include <macros.h>
#include <map>

class VMCBuddy
{
public:
    // Constructor accepting a pointer to the TaskScheduler
    VMCBuddy(Scheduler* scheduler, Print* serial = nullptr);
    void registerInterrupts();

    void setLed(uint8_t index, uint8_t value);
    void digitalWritePulse(uint8_t index, uint8_t value);
    void pinModePulse(uint8_t index, uint8_t mode);
    void setOnEventCallback(std::function<void(uint8_t, int)> callback);
    // Initialize the library
    void begin();

    void handleInterrupts(uint8_t pin);

    enum Event
    {
        BUTTON_PRESS,
        BUTTON_RELEASE,
        INPUT_RAISE,
        INPUT_FALL,
    };


private:
    Scheduler* scheduler;
    Print* serial;
    std::function<void(VMCBuddy::Event, int)> onEventCallback;
    uint8_t shiftRegisterBits[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t pulsePinMode[NUM_PULSE] = {0, 0, 0, 0, 0, 0, 0, 0};
    // std::map<uint8_t,uint8_t> inputValues;
    bool initialized = false;
    bool interruptHappened = false;

    Task* taskReadAnalogButtons;
    int oldAnalogButtonReadValue;
    int newAnalogButtonReadValue;

    void trigger(VMCBuddy::Event event, int value);
    void render();
    void setInterruptMode(uint8_t pin, uint8_t mode);
    void readAnalogButtons();
    void setShiftRegisterBit(uint8_t index, uint8_t value);

    // const uint8_t pinConfiguration[LEN_PIN_CONFIGURATION][2] = {
    std::map<uint8_t, uint8_t> pinConfiguration = {
        {PIN_MCU_SHIFTREGISTER_DATA, OUTPUT}, // 2 = Shift Register Data
        {PIN_MCU_SHIFTREGISTER_CLOCK, OUTPUT}, // 3 = Shift Register Clock
        {PIN_MCU_SHIFTREGISTER_LATCH, OUTPUT}, // 4 = Shift Register Latch
        {PIN_MCU_ANALOG_BTNS, INPUT},
        {PIN_MCU_PULSE_IN1, INPUT},
        {PIN_MCU_PULSE_IN2, INPUT},
        {PIN_MCU_PULSE_IN3, INPUT},
        {PIN_MCU_PULSE_IN4, INPUT},
        {PIN_MCU_PULSE_IN5, INPUT},
        {PIN_MCU_PULSE_IN6, INPUT},
        {PIN_MCU_PULSE_IN7, INPUT},
        {PIN_MCU_PULSE_IN8, INPUT},
    };

    // uint8_t pinInterrupts[LEN_PIN_INTERRUPTS][2] = {
    std::map<uint8_t, uint8_t> pinInterrupts = {
        {PIN_MCU_PULSE_IN1, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN2, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN3, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN4, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN5, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN6, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN7, DEFAULT_ISR_MODE},
        {PIN_MCU_PULSE_IN8, DEFAULT_ISR_MODE},
    };

    const uint8_t ledPins[NUM_LEDS] = {
        PIN_SR_LED1,
        PIN_SR_LED2,
        PIN_SR_LED3,
        PIN_SR_LED4,
        PIN_SR_LED5,
        PIN_SR_LED6,
    };

    const uint8_t pulseOutPins[NUM_PULSE] = {
        PIN_SR_PULSE_OUT1,
        PIN_SR_PULSE_OUT2,
        PIN_SR_PULSE_OUT3,
        PIN_SR_PULSE_OUT4,
        PIN_SR_PULSE_OUT5,
        PIN_SR_PULSE_OUT6,
        PIN_SR_PULSE_OUT7,
        PIN_SR_PULSE_OUT8,
    };
    const int analogButtonValues[NUM_BUTTONS] = {
        VAL_ANALOG_BTN1,
        VAL_ANALOG_BTN2,
        VAL_ANALOG_BTN3,
        VAL_ANALOG_BTN4,
        VAL_ANALOG_BTN5,
        VAL_ANALOG_BTN6,
    };
};

#endif // VMC_BUDDY_H
