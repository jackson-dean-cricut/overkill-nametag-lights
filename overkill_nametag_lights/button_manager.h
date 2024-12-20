#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <EventButton.h>
#include "events.h"

class ButtonManager {
public:
    static const int NUM_BUTTONS = 6;
    const int BUTTON_PINS[NUM_BUTTONS] = {14, 12, 13, 5, 4, 0};
    
    // Initialize the buttons in the constructor initialization list
    ButtonManager() : 
        buttons{
            EventButton(BUTTON_PINS[0]),
            EventButton(BUTTON_PINS[1]),
            EventButton(BUTTON_PINS[2]),
            EventButton(BUTTON_PINS[3]),
            EventButton(BUTTON_PINS[4]),
            EventButton(BUTTON_PINS[5])
        } 
    {}
    
    void begin();
    void update();

private:
    EventButton buttons[NUM_BUTTONS];
    
    void onButtonClicked(EventButton& btn);
    void onButtonDoubleClicked(EventButton& btn);
    void onButtonLongPress(EventButton& btn);
    void onButtonReleased(EventButton& btn);
};

#endif