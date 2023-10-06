#ifndef VIEW_H_
#define VIEW_H_

#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include "Root4root_VLC.h"

#define REDLED 5
#define GREENLED 6
#define BLUELED 7

#define DVOLTAGE 1
#define DRESISTANCE 2
#define DCURRENT 3

class View
{
private:
    LiquidCrystal_I2C lcd;
    Root4root_VLC<LiquidCrystal_I2C> vlc;
    char container[17] = {0};
    uint8_t previousDisplayMode{};
    void checkDisplayMode(uint8_t mode);
public:
    View();
    void init();
    void red();
    void green();
    void blue();
    void magenta();
    void black();
    void displayVoltage(uint16_t mV);
    void displayResistance(long ohms);
    void displayCurrent(float mA, bool newMeasure = false);
};


#endif /* VIEW_H_ */
