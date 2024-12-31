#include "state_manager.h"
#include "led_control.h"

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
        Serial.print("Animation: ");
        switch (animState.pattern) {
            case 0: Serial.println("0 - Rainbow"); break;
            case 1: Serial.println("1 - Wave"); break;
            case 2: Serial.println("2 - Pulse"); break;
            case 3: Serial.println("3 - Sparkle"); break;
            case 4: Serial.println("4 - Chase"); break;
            case 5: Serial.println("5 - Breathing"); break;
        }
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
        outputs[i].isOn = true;
        outputs[i].brightness = 255;
        // Each LED gets base hue plus its offset
        outputs[i].hue = animState.baseHue + outputs[i].animationOffset;
    }
}

void StateManager::updateWave() {
    animState.baseHue += animState.speed;
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        // Create a moving wave pattern using LEDUtils::sin8
        uint8_t wave = LEDUtils::sin8(animState.baseHue + outputs[i].animationOffset);
        outputs[i].brightness = wave;  // Use wave for brightness
        // Keep consistent colors per LED but shift them slowly
        outputs[i].hue = (outputs[i].animationOffset + (animState.baseHue / 4));
    }
}

void StateManager::updatePulse() {
    static float peakPosition = 0;
    static bool isRising = true;
    static uint8_t currentHue = 0;
    
    const uint8_t MIN_BRIGHTNESS = 20;
    int HUE_STEP = 1;  // How quickly to transition hue (higher = faster)

    // Update peak position
    float moveSpeed = 0.05f;
    if (isRising) {
        peakPosition += moveSpeed;
        if (peakPosition >= MAX_OUTPUTS - 1) {
            isRising = false;
        }
    } else {
        peakPosition -= moveSpeed;
        if (peakPosition <= -3) {
            isRising = true;
            currentHue += 20;
        }
    }

    // Update hues with smooth transition
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (outputs[i].hue != currentHue) {
            // Find shortest path to target hue (clockwise or counterclockwise)
            int diff = currentHue - outputs[i].hue;
            if (diff > 127) diff -= 256;
            else if (diff < -128) diff += 256;
            
            // Move hue closer to target by HUE_STEP
            if (diff > 0) {
                outputs[i].hue = outputs[i].hue + min(HUE_STEP, diff);
            } else if (diff < 0) {
                outputs[i].hue = outputs[i].hue - min(HUE_STEP, -diff);
            }
        }
    }

    // Update brightnesses based on distance from peak
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (i <= peakPosition) {
            outputs[i].brightness = 255;
        } else {
            float distance = i - peakPosition;
            float falloff = max(0.0f, 1.0f - (distance / 3.0f));
            uint8_t brightness = (uint8_t)(falloff * falloff * 255); // Square the falloff for gamma correction
            outputs[i].brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
        }
    }

    Serial.printf("%i, %i, %i, %i, %i, %i\n", outputs[0].brightness, outputs[1].brightness, outputs[2].brightness, outputs[3].brightness, outputs[4].brightness, outputs[5].brightness);
}

void StateManager::updateSparkle() {
    // Randomly update brightness and hue for random LEDs
    if (LEDUtils::random8() < 32) {  // Random sparkle probability
        int led = LEDUtils::random8(MAX_OUTPUTS);
        outputs[led].hue = LEDUtils::random8();
        outputs[led].brightness = LEDUtils::random8(128, 255);  // Brighter range for better visibility
    }
    
    // Gradually dim all LEDs
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        if (outputs[i].brightness > 8) {  // Gradual fade out
            outputs[i].brightness -= 8;
        }
    }
}

void StateManager::updateChase() {
    animState.baseHue += animState.speed;
    int active = (animState.baseHue >> 5) % MAX_OUTPUTS;
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        
        // Calculate distance from active LED (considering wrap-around)
        int distance = abs(i - active);
        if (distance > MAX_OUTPUTS/2) {
            distance = MAX_OUTPUTS - distance;
        }
        
        // Fade brightness based on distance from active LED
        outputs[i].brightness = distance == 0 ? 255 : (64 / (distance + 1));
        
        // Each LED maintains its base color but active one is brightest
        outputs[i].hue = outputs[i].animationOffset;
    }
}

void StateManager::updateBreathing() {
    animState.baseHue += animState.speed;
    uint8_t masterBrightness = LEDUtils::sin8(animState.baseHue);
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        // Each LED keeps its own color but breathes in sync
        outputs[i].hue = outputs[i].animationOffset;
        // Use wave pattern offset for each LED to create flowing breath
        uint8_t offsetBrightness = LEDUtils::sin8(masterBrightness + outputs[i].animationOffset/4);
        outputs[i].brightness = offsetBrightness;
    }
}