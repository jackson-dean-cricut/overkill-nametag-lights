#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// Button configuration
#define NUM_BUTTONS 6
extern const int BUTTON_PINS[NUM_BUTTONS];

// Timing configurations
#define LONG_PRESS_TIME 500    // 500ms for long press
#define DOUBLE_PRESS_TIME 300  // 300ms window for double press detection

// Button state tracking
extern bool buttonStates[NUM_BUTTONS];
extern bool buttonLongPress[NUM_BUTTONS];
extern bool buttonDoublePress[NUM_BUTTONS];

void setupButtons();
void updateButtons();
bool isButtonPressed(int buttonIndex);
bool isLongPress(int buttonIndex);
bool isDoublePress(int buttonIndex);

#endif