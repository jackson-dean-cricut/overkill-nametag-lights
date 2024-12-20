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
    bool isColorCycling;
    // For future networking: unsigned long lastUpdateTime;
};

class StateManager {
public:
    static const int MAX_OUTPUTS = 6;
    
    StateManager();
    
    // Core state management
    void toggleOutput(int index);
    void setColorCycling(int index, bool enabled);
    void updateHue(int index);
    const OutputState& getState(int index) const;
    bool isActive(int index) const;
    
private:
    OutputState outputs[MAX_OUTPUTS];
    // For future networking: bool stateChanged;
};

#endif // STATE_MANAGER_H