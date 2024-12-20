#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include <Shifty.h>

// Shift register configuration
#define SHIFT_DATA_PIN  16
#define SHIFT_CLOCK_PIN 15
#define NUM_OUTPUTS     8

extern Shifty shiftRegister;

void setupShiftRegister();
void updateShiftRegister(int index, bool state);
void updateAllShiftRegisters();

#endif