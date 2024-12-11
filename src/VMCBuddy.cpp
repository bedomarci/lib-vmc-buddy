#include "VMCBuddy.h"

// Constructor
VMCBuddy::VMCBuddy(Scheduler* scheduler, Print* serial)
    : scheduler(scheduler),
      serial(serial)
{
}

void VMCBuddy::registerInterrupts()
{
    for (const auto& pinInterrupt : pinInterrupts)
    {
        uint8_t pin = pinInterrupt.first;
        uint8_t mode = pinInterrupt.second;

        if (mode)
        {
            //TODO valahogy paramterezni kell az interrupotot
            //attachInterrupt(pin, [=]() {VMCBuddy::handleInterrupts(pin); }, mode);
        }
        else
        {
            detachInterrupt(pin);
        }
    }
}

// Initialization method
void VMCBuddy::begin()
{
    if (!this->serial)
    {
        this->serial = &Serial;
    }
    Log.begin(LOG_LEVEL_VERBOSE, this->serial);

    for (const auto& it : pinConfiguration)
    {
        pinMode(it.first, it.second);
    }

    for (uint8_t pin : ledPins)
        setLed(pin, LOW);

    registerInterrupts();


    taskReadAnalogButtons = new Task(TIME_ANALOG_BUTTON_READ_INTERVAL, TASK_FOREVER,
                                     [this]() { this->readAnalogButtons(); });
    this->scheduler->addTask(*this->taskReadAnalogButtons);
    this->taskReadAnalogButtons->enable();
    initialized = true;
}

void VMCBuddy::trigger(VMCBuddy::Event event, int value)
{
    if (this->onEventCallback)
    {
        onEventCallback(event, value);
    }
}

void VMCBuddy::render()
{
    digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, LOW);

    // Loop through the 16 bits
    for (int i = 0; i < LEN_SHIFT_REGISTER_BITS; ++i)
    {
        // Write the bit to the data pin
        digitalWrite(PIN_MCU_SHIFTREGISTER_DATA, shiftRegisterBits[i] ? HIGH : LOW);

        // Pulse the clock to shift the bit
        digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, HIGH);
        digitalWrite(PIN_MCU_SHIFTREGISTER_CLOCK, LOW);
    }

    // End transmission: Pull latch high
    digitalWrite(PIN_MCU_SHIFTREGISTER_LATCH, HIGH);
}

void VMCBuddy::handleInterrupts(uint8_t pin)
{
    for (const auto& pinInterrupt : this->pinInterrupts)
    {
        // inputValues[pin] = digitalRead(pin);
        //TODO azokat az elemeket, amik valtoznak, meg kell jelolni dirtynek, hogy kesobb egy TASK fel tudja dolgozni a valtozasokat.
    }
    // trigger((value) ? INPUT_RAISE : INPUT_FALL, pin);
}

void VMCBuddy::setInterruptMode(const uint8_t pin, const uint8_t mode)
{
    if (initialized)
    {
        Log.errorln("Interrupts must be registered before initialiyation. Move setInterruptMode() above begin(). ");
        return;
    }
    pinInterrupts[pin] = mode;
}

void VMCBuddy::readAnalogButtons()
{
    this->newAnalogButtonReadValue = analogRead(PIN_MCU_ANALOG_BTNS);
    int delta = abs(this->newAnalogButtonReadValue - this->oldAnalogButtonReadValue);
    if (delta > TOLERANCE_ANALOG_BUTTONS_CHANGE)
    {
        if (this->newAnalogButtonReadValue < TOLERANCE_ANALOG_BUTTONS_CHANGE)
        {
            trigger(BUTTON_RELEASE, 0);
        }
        else
        {
            for (int i = 0; i < NUM_BUTTONS; i++)
            {
                if (this->newAnalogButtonReadValue > VMCBuddy::analogButtonValues[i] - TOLERANCE_ANALOG_BUTTONS_CHANGE
                    &&
                    this->newAnalogButtonReadValue < VMCBuddy::analogButtonValues[i] + TOLERANCE_ANALOG_BUTTONS_CHANGE)
                {
                    trigger(BUTTON_PRESS, i);
                }
            }
        }
    }
    this->oldAnalogButtonReadValue = this->newAnalogButtonReadValue;
}

void VMCBuddy::setOnEventCallback(std::function<void(uint8_t, int)> callback)
{
    this->onEventCallback = callback;
}

void VMCBuddy::setShiftRegisterBit(uint8_t index, uint8_t value)
{
    this->shiftRegisterBits[index] = value;
    this->render();
}

void VMCBuddy::setLed(uint8_t index, uint8_t value)
{
    index = constrain(index, 1, NUM_LEDS) - 1;
    this->setShiftRegisterBit(this->ledPins[index], !value);
}

void VMCBuddy::digitalWritePulse(uint8_t index, uint8_t value)
{
    index = constrain(index, 1, NUM_PULSE) - 1;
    if (pulsePinMode[index] == INPUT) return;
    this->setShiftRegisterBit(this->pulseOutPins[index], value);
}

void VMCBuddy::pinModePulse(uint8_t index, uint8_t mode)
{
    index = constrain(index, 1, NUM_PULSE) - 1;
    pulsePinMode[index] = mode;

    if (mode == INPUT)
    {
        setShiftRegisterBit(pulseOutPins[index], 0);
    }
}
