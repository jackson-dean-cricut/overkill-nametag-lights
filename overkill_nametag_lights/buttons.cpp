#include "buttons.h"

const int BUTTON_PINS[NUM_BUTTONS] = {14, 12, 13, 5, 4, 0};
bool buttonStates[NUM_BUTTONS] = {false};
bool buttonLongPress[NUM_BUTTONS] = {false};
bool buttonDoublePress[NUM_BUTTONS] = {false};

// Internal state tracking
static unsigned long buttonPressTime[NUM_BUTTONS] = {0};
static unsigned long lastReleaseTime[NUM_BUTTONS] = {0};
static unsigned long pressCount[NUM_BUTTONS] = {0};

void setupButtons() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
        lastReleaseTime[i] = 0;
        pressCount[i] = 0;
    }
}

void updateButtons() {
    unsigned long currentTime = millis();
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool currentState = !digitalRead(BUTTON_PINS[i]);
        
        // Button just pressed
        if (currentState && !buttonStates[i]) {
            buttonPressTime[i] = currentTime;
            buttonLongPress[i] = false;
            
            // Check for double press - if second press happens within window
            if (pressCount[i] == 1 && (currentTime - lastReleaseTime[i] < DOUBLE_PRESS_TIME)) {
                Serial.println("Double press");
                buttonDoublePress[i] = true;
                pressCount[i] = 0;  // Reset count after double press detected
            }
        }
        
        // Button just released
        if (!currentState && buttonStates[i]) {
            lastReleaseTime[i] = currentTime;
            pressCount[i]++;  // Increment press count on every release
            
            if (buttonDoublePress[i]) {
                buttonDoublePress[i] = false;  // Clear double press on release
                pressCount[i] = 0;  // Reset counter after double press handled
            }
        }
        
        // Check for long press
        if (currentState && buttonStates[i]) {
            if (currentTime - buttonPressTime[i] > LONG_PRESS_TIME) {
                buttonLongPress[i] = true;
                pressCount[i] = 0;  // Reset count on long press
                buttonDoublePress[i] = false;  // Cancel any pending double press
            }
        }
        
        // Clear old press counts
        if (currentTime - lastReleaseTime[i] > DOUBLE_PRESS_TIME && !currentState) {
            pressCount[i] = 0;
        }
        
        buttonStates[i] = currentState;
    }
}

bool isButtonPressed(int buttonIndex) {
    if (buttonIndex >= NUM_BUTTONS) return false;
    return buttonStates[buttonIndex];
}

bool isLongPress(int buttonIndex) {
    if (buttonIndex >= NUM_BUTTONS) return false;
    return buttonLongPress[buttonIndex];
}

bool isDoublePress(int buttonIndex) {
    if (buttonIndex >= NUM_BUTTONS) return false;
    return buttonDoublePress[buttonIndex];
}