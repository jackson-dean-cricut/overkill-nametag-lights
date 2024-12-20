#include "output_manager.h"
#include "led_control.h"
#include "shift_register.h"
#include "button_manager.h"

void OutputManager::begin(StateManager& sm) {
    stateManager = &sm;
    setupLEDs();
    setupShiftRegister();
}

void OutputManager::update() {
    // Update physical outputs based on current state
    for (int i = 0; i < ButtonManager::NUM_BUTTONS; i++) {
        const OutputState& state = stateManager->getState(i);
        if (stateManager->isInAnimationMode()) {
            // WS2811 LEDs show the animation
            updateLED(i, state.isOn, state.hue);
            // Shift register LEDs show which animation is active
            updateShiftRegister(i + 1, (i == stateManager->getAnimationPattern()));
        } else {
            // Normal mode - both LED types show button state
            updateLED(i, state.isOn, state.hue);
            updateShiftRegister(i + 1, state.isOn);
        }
    }
    
    showLEDs();
    updateAllShiftRegisters();
}