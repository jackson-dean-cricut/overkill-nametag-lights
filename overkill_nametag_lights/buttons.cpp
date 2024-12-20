#include "buttons.h"

const int BUTTON_PINS[NUM_BUTTONS] = {14, 12, 13, 5, 4, 0};
bool buttonStates[NUM_BUTTONS] = {false};
bool buttonLongPress[NUM_BUTTONS] = {false};
unsigned long buttonPressTime[NUM_BUTTONS] = {0};

void setupButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
}

void updateButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool currentState = !digitalRead(BUTTON_PINS[i]);
    
    // Button just pressed
    if (currentState && !buttonStates[i]) {
      buttonPressTime[i] = millis();
      buttonLongPress[i] = false;
    }
    
    // Check for long press
    if (currentState && buttonStates[i]) {
      if (millis() - buttonPressTime[i] > LONG_PRESS_TIME) {
        buttonLongPress[i] = true;
      }
    }
    
    buttonStates[i] = currentState;
  }
}

bool isButtonPressed(int buttonIndex) {
  return buttonStates[buttonIndex];
}

bool isLongPress(int buttonIndex) {
  return buttonLongPress[buttonIndex];
}