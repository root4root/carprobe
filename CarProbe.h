#ifndef CARPROBE_H_
#define CARPROBE_H_

#define REFERENCE 5.016

#define RELAY 2
#define PFET 3
#define PROBE 4
#define PBUTTON 10

#define BUTTON_DEBOUNCE_TIMEOUT 250

Root4root_INA219 ina219; //0x40 address
View view;

void measureResistance();
bool powerButton();
void autoMode();


#endif /* CARPROBE_H_ */
