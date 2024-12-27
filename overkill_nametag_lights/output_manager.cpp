#include "output_manager.h"
#include "button_manager.h"

void OutputManager::begin(StateManager& sm) {
    stateManager = &sm;
    ledController.begin();
    shiftRegister.begin();
}

void OutputManager::update() {
    // Update physical outputs based on current state
    for (int i = 0; i < ButtonManager::NUM_BUTTONS; i++) {
        const OutputState& state = stateManager->getState(i);
        if (stateManager->isInAnimationMode()) {
            // WS2811 LEDs show the animation
            ledController.updateLED(i, state.isOn, state.hue);
            // Shift register LEDs show which animation is active
            shiftRegister.updateRegister(i + 1, (i == stateManager->getAnimationPattern()));
        } else {
            // Normal mode - both LED types show button state
            ledController.updateLED(i, state.isOn, state.hue);
            shiftRegister.updateRegister(i + 1, state.isOn);
        }
    }
    
    ledController.show();
    shiftRegister.updateAll();
}