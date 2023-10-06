#ifndef CARPROBE_H_
#define CARPROBE_H_

#define REFERENCE 5.016

#define RELAY 2
#define PFET 3
#define PROBE 4
#define POWER_BUTTON 10

//About power button
#define PRESSED_BUTTON_STATE 0
#define BUTTON_PUSH_DURATION 100 //ms

Root4root_INA219 ina219; //0x40 address
View view;

void measureResistance();
bool powerButtonHandler();
void autoMode();
bool readButtonRoutine();

#endif /* CARPROBE_H_ */
