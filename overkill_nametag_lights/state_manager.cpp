#include "state_manager.h"
#include <FastLED.h>

StateManager::StateManager() {
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i] = {false, 0, 255, false, 0};  // Default to full brightness
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

void StateManager::update() {
    // Handle animation mode updates
    if (animState.isAnimating) {
        updateAnimations();
        return;  // Skip individual updates in animation mode
    }
    
    // Handle individual LED updates (color cycling)
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (outputs[i].isColorCycling && outputs[i].isOn) {
            updateHue(i);
        }
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
    outputs[index] = {false, 0, 255, false, 0};  // Reset to default state
    // For future networking: stateChanged = true;
}

void StateManager::toggleAnimationMode() {
    animState.isAnimating = !animState.isAnimating;
    if (animState.isAnimating) {
        // Reset all outputs for animation mode
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            outputs[i].isOn = true;
            outputs[i].isColorCycling = false;
            outputs[i].brightness = 255;
            // Space the LEDs evenly around the color wheel
            outputs[i].animationOffset = (i * 256) / MAX_OUTPUTS;
        }
        animState.pattern = 0;  // Start with rainbow pattern
    } else {
        // Reset all outputs to off when exiting animation mode
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            resetOutput(i);
        }
    }
}

void StateManager::setAnimationPattern(uint8_t pattern) {
    if (pattern < NUM_PATTERNS) {
        animState.pattern = pattern;
    }
}

uint8_t StateManager::getAnimationPattern() {
    return animState.pattern;
}

void StateManager::updateAnimations() {
    if (!animState.isAnimating) return;
    
    switch (animState.pattern) {
        case 0: updateRainbow(); break;
        case 1: updateWave(); break;
        case 2: updatePulse(); break;
        case 3: updateSparkle(); break;
        case 4: updateChase(); break;
        case 5: updateBreathing(); break;
    }
}

// Animation pattern implementations
void StateManager::updateRainbow() {
    animState.baseHue += animState.speed;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].hue = animState.baseHue + outputs[i].animationOffset;
        outputs[i].isOn = true;
    }
}

void StateManager::updateWave() {
    animState.baseHue += animState.speed;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        uint8_t wave = sin8(animState.baseHue + outputs[i].animationOffset);
        outputs[i].hue = wave;
        outputs[i].isOn = true;
    }
}

void StateManager::updatePulse() {
    animState.baseHue += animState.speed;
    uint8_t pulse = sin8(animState.baseHue);
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].hue = pulse + outputs[i].animationOffset;
        outputs[i].isOn = true;
    }
}

void StateManager::updateSparkle() {
    if (random8() < 32) {  // Random sparkle probability
        int led = random8(MAX_OUTPUTS);
        outputs[led].hue = random8();
    }
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
    }
}

void StateManager::updateChase() {
    animState.baseHue += animState.speed;
    int active = (animState.baseHue >> 5) % MAX_OUTPUTS;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = (i == active);
        if (outputs[i].isOn) {
            outputs[i].hue = animState.baseHue;
        }
    }
}

void StateManager::updateBreathing() {
    animState.baseHue += animState.speed;
    uint8_t brightness = sin8(animState.baseHue);
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = (brightness > 32);  // Turn off at very low brightness
        outputs[i].hue = outputs[i].animationOffset;
    }
}