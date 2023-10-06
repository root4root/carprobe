#include <Arduino.h>
#include "Root4root_INA219.h"
#include "View.h"
#include "CarProbe.h"

void setup()
{
    pinMode(RELAY, OUTPUT);           //Power activation pin.
    pinMode(PFET, OUTPUT);            //Power activation pin.
    pinMode(PROBE, OUTPUT);           //Ohm Ω meter pin
    pinMode(POWER_BUTTON, INPUT);     //Power activation button
    digitalWrite(POWER_BUTTON, HIGH); //Enable internal pull-up resistor

    digitalWrite(RELAY, LOW);   //External power relay (30A)
    digitalWrite(PFET,  LOW);   //Internal PFET relay
    digitalWrite(PROBE, LOW);   //Probe pin

    ina219.begin(20000, 10);    //Calibration, max expected current 20A, shunt resistance in mOhm

    ina219.changeConfig(
        INA219_CONFIG_SHUNT_ADC_RESOLUTION_12BIT_8S_4260US,
        INA219_CONFIG_SHUNT_ADC_RESOLUTION_MASK
    );

    view.init();
}

void loop() {

    if (powerButtonHandler()) {
        return; //Go to the next loop() iteration
    }

    autoMode();
}

void autoMode()
{
    digitalWrite(PROBE, LOW);

    uint16_t voltage = ina219.getBusVoltage_mV();

    if (voltage < 5) { //Less than 5 mV !
        measureResistance();
    } else {
        view.displayVoltage(voltage);
    }
}

void measureResistance()
{
    digitalWrite(PROBE, HIGH);

    uint16_t voltage = ina219.getBusVoltage_mV();

    if (voltage < 2) {
        view.displayResistance(0);
        return;
    }

    if (voltage >= 5000) {
        view.displayResistance(-1); //Infinite - any value less than 0
        return;
    }

    //R2=VoR1/(Vs-Vo);
    //Since resistance is 1000 Ohm we keep voltage in mV
    view.displayResistance(voltage/(REFERENCE - voltage/1000.0));
}

bool powerButtonHandler()
{
    static bool currentOutputState = false;

    bool currentButtonState = readButtonRoutine();

    if (currentOutputState == true) {
        view.displayCurrent(ina219.getCurrent_mA());
    }

    if (currentOutputState == currentButtonState) {
        return currentOutputState;
    }

    if (currentButtonState == true) {
        digitalWrite(PROBE, LOW);
        digitalWrite(PFET, HIGH);
        view.magenta();
        view.displayCurrent(ina219.getCurrent_mA(), true);
    } else {
        digitalWrite(PFET, LOW);
        view.black();
    }

    currentOutputState = currentButtonState;

    return currentOutputState;
}

bool readButtonRoutine() //strict EMI protection and debounce
{
    static uint16_t pushDuration = 0;
    static uint32_t previousTime = millis();   //in case multiple buttons, could be moved level up
    static bool normalizedButtonState = false; //which'll be returned to the caller

    bool rawButtonState = digitalRead(POWER_BUTTON);

    //unlock pushDuration timer, start action if button pressed
    if (rawButtonState == PRESSED_BUTTON_STATE && pushDuration == 0) {
        ++pushDuration;
        previousTime = millis();
    }

    if (pushDuration == 0) { //idle guard...
        return normalizedButtonState;
    }

    uint16_t timeDifference = millis() - previousTime;

    if (timeDifference == 0) { //minimum time chunk is 1ms
        return normalizedButtonState;
    }

    previousTime = millis();

    //EMI protection, including severe ones
    if (rawButtonState == PRESSED_BUTTON_STATE) {
        uint16_t tempResult = pushDuration + timeDifference;
        pushDuration = (tempResult > BUTTON_PUSH_DURATION) ? BUTTON_PUSH_DURATION : tempResult;
    } else {
        ++timeDifference; //decrease slightly faster to gain EMI protection even more...
        pushDuration -= (timeDifference > pushDuration) ? pushDuration : timeDifference;
    }

    if (pushDuration == BUTTON_PUSH_DURATION) {
        normalizedButtonState = true;
    }

    if (pushDuration == 0) {
        normalizedButtonState = false;
    }

    return normalizedButtonState;
}
