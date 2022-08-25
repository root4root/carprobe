#include "Arduino.h"
#include "Root4root_INA219.h"
#include "View.h"
#include "CarProbe.h"

void setup()
{
    pinMode(RELAY, OUTPUT);          //Power activation pin.
    pinMode(PFET, OUTPUT);           //Power activation pin.
    pinMode(PROBE, OUTPUT);          //Ohm meter pin
    pinMode(PBUTTON, INPUT);         //Power activation button
    digitalWrite(PBUTTON, HIGH);     //Enable internal pull-up resistor

    digitalWrite(RELAY, LOW);  //External power relay (30A)
    digitalWrite(PFET,  LOW);   //Internal PFET relay
    digitalWrite(PROBE, LOW);  //Probe pin

    ina219.begin(20000, 10); //Calibration, max expected current 20A, shunt resistance in mOhm
    ina219.changeConfig(INA219_CONFIG_SHUNT_ADC_RESOLUTION_12BIT_8S_4260US, INA219_CONFIG_SHUNT_ADC_RESOLUTION_MASK);

    view.init();
}

void loop() {

    if (powerButton()) {
        return; //Go to the next loop() iteration i.e. skip code below.
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

bool powerButton()
{
    static bool currentState = false;
    static unsigned long previuosChangeTime = 0; //Debounce timer

    bool button = false;

    if (currentState == true) {
        view.displayCurrent(ina219.getCurrent_mA());
    }

    button = !digitalRead(10); //Inverse due to normalize ON = true OFF = false;

    if ((millis() - previuosChangeTime) < BUTTON_DEBOUNCE_TIMEOUT || currentState == button) {
        return currentState;
    }

    if (button) {
        digitalWrite(PROBE, LOW);
        digitalWrite(PFET, HIGH);
        view.displayCurrent(ina219.getCurrent_mA(), true);
    } else {
        digitalWrite(PFET, LOW);
    }

    currentState = button;
    previuosChangeTime = millis();

    return currentState;
}
