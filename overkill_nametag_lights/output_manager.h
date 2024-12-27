#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include "events.h"
#include "state_manager.h"
#include "led_control.h"
#include "shift_register.h"

class OutputManager {
public:
    void begin(StateManager& stateManager);
    void update();  // Call this in loop() to update physical outputs

private:
    StateManager* stateManager;
    LEDController ledController;
    ShiftRegisterController shiftRegister;
};

#endif