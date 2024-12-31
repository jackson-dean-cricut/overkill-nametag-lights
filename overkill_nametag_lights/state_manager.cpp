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
    static unsigned long lastUpdate = 0;
    const unsigned long UPDATE_INTERVAL = 20;  // Same as original delay
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate < UPDATE_INTERVAL) {
        return;  // Not time to update yet
    }

    lastUpdate = currentMillis;

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
    static float offset = 0;
    const float RAINBOW_SPEED = 0.2f;
    
    offset = fmod(offset + RAINBOW_SPEED * animState.speed, 256);
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        outputs[i].brightness = 255;
        outputs[i].hue = (uint8_t)(offset + (i * 256.0f / MAX_OUTPUTS));
    }
}

void StateManager::updateWave() {
    static float wavePosition = 0;
    static float lastWavePosition = 0;
    static uint8_t targetHues[MAX_OUTPUTS];
    static bool huesInitialized = false;
    
    const float WAVE_SPEED = 0.015f;    // Speed of wave movement
    const float WAVE_WIDTH = 2.0f * PI / MAX_OUTPUTS;    // How spread out the wave is
    const int16_t HUE_STEP = 1;       // How quickly hues shift
    
    // Initialize random target hues if first run
    if (!huesInitialized) {
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            outputs[i].hue = random(256);
            targetHues[i] = outputs[i].hue;
        }
        huesInitialized = true;
        lastWavePosition = wavePosition;
    }
    
    // Update wave position
    wavePosition = fmod(wavePosition + WAVE_SPEED * animState.speed, MAX_OUTPUTS);
    
    // Check if wave has passed any LEDs
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if ((lastWavePosition < i && wavePosition >= i) || 
            (lastWavePosition > wavePosition && (lastWavePosition < i || wavePosition >= i))) {
            outputs[i].hue = random(256); // new color as wave passes
        }
        
        outputs[i].isOn = true;
        
        // Calculate wave brightness using distance from wave peak
        float distance = abs((float)i - wavePosition);
        if (distance > MAX_OUTPUTS/2) {
            distance = MAX_OUTPUTS - distance;
        }
        Serial.printf("%.2f ", distance);
        float brightness = cos(distance * WAVE_WIDTH);
        brightness = 1.0f - ((brightness + 1.0f) * 0.5f);  //  normalized wave
        outputs[i].brightness = (uint8_t)(brightness * 255);
        
    }
    // Serial.printf("%i, %i, %i, %i, %i, %i\n", outputs[0].brightness, outputs[1].brightness, outputs[2].brightness, outputs[3].brightness, outputs[4].brightness, outputs[5].brightness);

    lastWavePosition = wavePosition;
}

void StateManager::updatePulse() {
    static float peakPosition = 0;
    static bool isRising = true;
    static uint8_t currentHue = 0;
    
    const uint8_t MIN_BRIGHTNESS = 1;
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
            uint8_t brightness = (uint8_t)(falloff * falloff * 255);
            outputs[i].brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
        }
    }

    // Serial.printf("%i, %i, %i, %i, %i, %i\n", outputs[0].brightness, outputs[1].brightness, outputs[2].brightness, outputs[3].brightness, outputs[4].brightness, outputs[5].brightness);
}

void StateManager::updateSparkle() {
    // Randomly update brightness and hue for random LEDs
    if (LEDUtils::random8() < 16) {  // Random sparkle probability
        int led = LEDUtils::random8(MAX_OUTPUTS);
        outputs[led].hue = LEDUtils::random8();
        outputs[led].brightness = LEDUtils::random8(128, 255);  // Brighter range for better visibility
    }
    
    // Gradually dim all LEDs
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        if (outputs[i].brightness > 0) {  // Gradual fade out
            outputs[i].brightness = max((int)outputs[i].brightness - 2, 0);
        }
    }
}

void StateManager::updateChase() {
    static float peakPosition = 0;
    static uint8_t currentHue = 0;
    
    const float MOVE_SPEED = 0.01f;  // Adjust for desired speed
    const float FALLOFF_DISTANCE = 2.0f;  // How many LEDs to spread the falloff over
    const uint8_t MIN_BRIGHTNESS = 0;  // Minimum brightness in the falloff
    const uint8_t HUE_STEP = 1;  // How quickly the hue changes
    
    // Update peak position with wrapping
    peakPosition = fmod(peakPosition + MOVE_SPEED * animState.speed, MAX_OUTPUTS);
    
    // Gradually update the hue
    currentHue += HUE_STEP;
    
    // Update all LEDs
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        
        // Calculate shortest distance to peak, considering wrap-around
        float distance = abs(i - peakPosition);
        if (distance > MAX_OUTPUTS/2) {
            distance = MAX_OUTPUTS - distance;
        }
        
        // Calculate brightness based on distance from peak
        float falloff;
        if (distance <= FALLOFF_DISTANCE) {
            // Quadratic falloff for smoother transition
            falloff = 1.0f - (distance / FALLOFF_DISTANCE);
            falloff = falloff * falloff;
            
            // Sharper leading edge when moving forward
            if (i > peakPosition && i < peakPosition + FALLOFF_DISTANCE) {
                falloff *= 0.1f;  // Reduce brightness on leading edge
            }
        } else {
            falloff = 0.0f;
        }
        
        outputs[i].brightness = constrain((uint8_t)(falloff * 255), MIN_BRIGHTNESS, 255);
        outputs[i].hue = currentHue;  // All LEDs share the same gradually shifting hue
    }
}

void StateManager::updateBreathing() {
    static float breathPosition = 0;
    static uint8_t targetHues[MAX_OUTPUTS];
    static bool huesInitialized = false;
    
    const float BREATH_SPEED = 0.02f;  // Adjust for desired breath rate
    const int HUE_STEP = 1;        // How quickly hues shift
    const uint8_t HUE_VARIATION = 30;   // Max random hue shift when picking new target
    
    // Initialize random target hues if first run
    if (!huesInitialized) {
        for (int i = 0; i < MAX_OUTPUTS; i++) {
            outputs[i].hue = random(256);
            targetHues[i] = outputs[i].hue;
        }
        huesInitialized = true;
    }
    
    // Update breath position (0 to 2π)
    breathPosition = fmod(breathPosition + BREATH_SPEED * animState.speed, 2 * PI);
    
    // Calculate master brightness using sine wave
    float masterBrightness = (sin(breathPosition) + 1.0f) * 0.5f;  // Normalized to 0.0 - 1.0
    
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        outputs[i].isOn = true;
        
        // Randomly update target hue occasionally
        if (random(100) < 2) {  // 2% chance each update
            int8_t shift = random(-HUE_VARIATION, HUE_VARIATION + 1);
            targetHues[i] = outputs[i].hue + shift;
        }
        
        // Gradually shift current hue toward target
        if (outputs[i].hue != targetHues[i]) {
            // Find shortest path to target hue
            int diff = targetHues[i] - outputs[i].hue;
            if (diff > 127) diff -= 256;
            else if (diff < -128) diff += 256;
            
            // Move hue closer to target
            if (diff > 0) {
                outputs[i].hue = outputs[i].hue + min(HUE_STEP, diff);
            } else if (diff < 0) {
                outputs[i].hue = outputs[i].hue - min(HUE_STEP, -diff);
            }
        }
        
        // Calculate brightness with slight offset for each LED
        float offsetPhase = breathPosition + (float)i * 0.2f;  // Adjust 0.2 for more/less offset
        float brightness = (sin(offsetPhase) + 1.0f) * 0.5f;
        
        // Apply gamma correction for smoother brightness transitions
        brightness = brightness * brightness;  // Square for gamma correction
        
        outputs[i].brightness = (uint8_t)(brightness * 255);
    }
}