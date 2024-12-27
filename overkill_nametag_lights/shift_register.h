#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include <Shifty.h>

class ShiftRegisterController {
public:
    static const int NUM_OUTPUTS = 8;
    static const int SHIFT_DATA_PIN = 16;
    static const int SHIFT_CLOCK_PIN = 15;

    ShiftRegisterController();
    void begin();
    void updateRegister(int index, bool state);
    void updateAll();

private:
    Shifty shiftRegister;
    bool shiftStates[NUM_OUTPUTS];
};

#endif