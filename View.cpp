#include "View.h"
#include <Arduino.h>

View::View(): lcd(0x27,16,2), vlc(&lcd, 16, 1, 0, 1){}

void View::init()
{
    pinMode(REDLED, OUTPUT); //Red 200 Ohm 14 mA
    pinMode(GREENLED, OUTPUT); //Green 200 Ohm 7.5 mA
    pinMode(BLUELED, OUTPUT); //Blue 200 Ohm 7.5 mA

    vlc.setDelayTime(333);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print(F("Auto mode:"));
}

void View::displayVoltage(uint16_t mV)
{
    checkDisplayMode(DVOLTAGE);
    previousDisplayMode = DVOLTAGE;

    char fvalue[6] = {0};

    this->red();

    dtostrf(mV/1000.0, 5, 2, fvalue);
    sprintf(container, "%14s %c", fvalue, 'V');
    vlc.displayString(container);
}

void View::displayResistance(long ohms)
{
    checkDisplayMode(DRESISTANCE);
    previousDisplayMode = DRESISTANCE;

    char fvalue[8] = {0};

    if (ohms < 0) {
        this->black();
        sprintf(container, "%13s%c %c", "", 0xF3, 0xF4);
        vlc.displayString(container);
        return;
    }

    if (ohms == 0) {
        this->blue();
    } else {
        this->green();
    }

    if (ohms > 1000) {
        dtostrf(ohms/1000.0, 7, 2, fvalue);
        sprintf(container, "%13s K%c", fvalue, 0xF4);
        vlc.displayString(container);
        return;
    }

    sprintf(container, "%14lu %c", ohms, 0xF4);
    vlc.displayString(container);
}

void View::displayCurrent(float mA, bool newMeasure = false)
{
    checkDisplayMode(DCURRENT);
    previousDisplayMode = DCURRENT;

    static int16_t avarage[5] = {};
    static unsigned long previuosChangeTime = 0; //Debounce

    uint8_t i = 0;
    int16_t result = 0;

    if (newMeasure) {
        for (i = 0; i < 5; ++i) {
            avarage[i] = 0;
        }
    }

    for (i = 0; i < 4; ++i) {
        avarage[i] = avarage[i+1];
        result += avarage[i];
    }

    avarage[4] = mA;
    result += mA;
    result /= 5;

    if (result > 10000) {
        result /= 1000;
        sprintf(container, "%14d %c", result, 'A');
    } else {
        sprintf(container, "%13d %s", result, "mA");
    }

    vlc.displayString(container);
}

void View::red()
{
    digitalWrite(REDLED, HIGH);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, LOW);
}

void View::green()
{
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, HIGH);
    digitalWrite(BLUELED, LOW);
}

void View::blue()
{
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, HIGH);
}

void View::magenta()
{
    digitalWrite(REDLED, HIGH);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, HIGH);
}

void View::black()
{
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, LOW);
}

void View::checkDisplayMode(uint8_t currentMode)
{
    if (previousDisplayMode != currentMode) {
        vlc.resetDelayCounter();
    }
}
