#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

/*
Future Networking Implementation Notes:
- Add ArduinoJson library for state serialization
- Implement state serialization methods (toJson(), fromJson())
- Add timestamps to OutputState for sync
- Add methods for partial state updates
- Consider adding:
  * Device ID for multi-device identification
  * Sequence numbers for detecting missed updates
  * Command queue for time-synchronized changes
*/

struct OutputState {
    bool isOn;
    uint8_t hue;
    uint8_t brightness;
    bool isColorCycling;
    uint8_t animationOffset;    // Offset from base hue for animations
    // For future networking: unsigned long lastUpdateTime;
};

struct AnimationState {
    uint8_t baseHue = 0;        // Base hue for animations
    uint8_t speed = 2;          // Animation speed
    uint8_t pattern = 0;        // Current animation pattern
    bool isAnimating = false;   // Global animation mode flag
};

class StateManager {
public:
    static const int MAX_OUTPUTS = 6;
    static const int NUM_PATTERNS = 6;  // Number of animation patterns
    
    StateManager();
    
    // Core state management
    void toggleOutput(int index);
    void setColorCycling(int index, bool enabled);
    void updateHue(int index);
    void update();
    void resetOutput(int index);
    const OutputState& getState(int index) const;
    bool isActive(int index) const;
    // for future networking: bool stateChanged;
    
    // Animation management
    void toggleAnimationMode();
    void setAnimationPattern(uint8_t pattern);
    uint8_t getAnimationPattern();
    void updateAnimations();
    bool isInAnimationMode() const { return animState.isAnimating; }
    
private:
    OutputState outputs[MAX_OUTPUTS];
    AnimationState animState;
    
    // Animation patterns
    void updateRainbow();
    void updateWave();
    void updatePulse();
    void updateSparkle();
    void updateChase();
    void updateBreathing();
};

#endif // STATE_MANAGER_H