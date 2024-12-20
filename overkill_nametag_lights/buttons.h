#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// Button configuration
#define NUM_BUTTONS 6
extern const int BUTTON_PINS[NUM_BUTTONS];

// Long press timing
#define LONG_PRESS_TIME 500 // 500ms for long press

// Button state tracking
extern bool buttonStates[NUM_BUTTONS];
extern bool buttonLongPress[NUM_BUTTONS];

void setupButtons();
void updateButtons();
bool isButtonPressed(int buttonIndex);
bool isLongPress(int buttonIndex);

#endif