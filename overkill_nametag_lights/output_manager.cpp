#include "output_manager.h"
#include "button_manager.h"

void OutputManager::begin(StateManager& sm) {
    stateManager = &sm;
    ledController.begin();
    shiftRegister.begin();
}

void OutputManager::update() {
    for (int i = 0; i < ButtonManager::NUM_BUTTONS; i++) {
        const OutputState& state = stateManager->getState(i);
        if (stateManager->isInAnimationMode()) {
            ledController.updateLED(i, state.isOn, state.hue, state.brightness);
            shiftRegister.updateRegister(i + 1, (i == stateManager->getAnimationPattern()));
        } else {
            ledController.updateLED(i, state.isOn, state.hue, state.brightness);
            shiftRegister.updateRegister(i + 1, state.isOn);
        }
    }
    
    ledController.show();
    shiftRegister.updateAll();
}