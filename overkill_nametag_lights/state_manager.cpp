#include "state_manager.h"

StateManager::StateManager() {
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i] = {false, 0, false};
    }
}

void StateManager::toggleOutput(int index) {
    if (index >= MAX_OUTPUTS) return;
    outputs[index].isOn = !outputs[index].isOn;
    // For future networking: stateChanged = true;
}

void StateManager::setColorCycling(int index, bool enabled) {
    if (index >= MAX_OUTPUTS) return;
    outputs[index].isColorCycling = enabled;
    // For future networking: stateChanged = true;
}

void StateManager::updateHue(int index) {
    if (index >= MAX_OUTPUTS) return;
    if (outputs[index].isColorCycling) {
        outputs[index].hue += 2;
        // For future networking: stateChanged = true;
    }
}

const OutputState& StateManager::getState(int index) const {
    static OutputState defaultState = {false, 0, false};
    if (index >= MAX_OUTPUTS) return defaultState;
    return outputs[index];
}

bool StateManager::isActive(int index) const {
    if (index >= MAX_OUTPUTS) return false;
    return outputs[index].isOn;
}

void StateManager::resetOutput(int index) {
    if (index >= MAX_OUTPUTS) return;
    outputs[index] = {false, 0, false};  // Reset to default state
    // For future networking: stateChanged = true;
}