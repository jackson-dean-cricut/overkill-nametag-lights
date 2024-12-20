#include <Arduino.h>
#include "buttons.h"
#include "led_control.h"
#include "shift_register.h"
#include "state_manager.h"

/*
Future Networking Implementation Notes:
1. Add NetworkManager class to handle WiFi/ESP-NOW
2. In setup():
   - Initialize WiFi or ESP-NOW
   - Set device role (controller/follower)
3. In loop():
   - Check for network messages
   - Synchronize state with other devices
   - Add small random delay for network message handling

Example network message structure:
{
  "deviceId": "tag1",
  "sequence": 123,
  "timestamp": 1234567890,
  "states": [{
    "index": 0,
    "isOn": true,
    "hue": 128,
    "isColorCycling": false
  },
  ...
  ]
}
*/

StateManager stateManager;
bool prevButtonStates[NUM_BUTTONS] = {false};
bool prevLongPress[NUM_BUTTONS] = {false};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");
    
    setupButtons();
    setupLEDs();
    setupShiftRegister();
    
    delay(2000);  // Initial delay for programming
}

void loop() {
    // Handle button inputs
    updateButtons();
    
    // Update state based on button inputs
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool currentPress = isButtonPressed(i);
        bool currentLongPress = isLongPress(i);
        bool currentDoublePress = isDoublePress(i);
        
        // Handle button release
        if (!currentPress && prevButtonStates[i]) {
            if (!prevLongPress[i] && !currentDoublePress) {
                Serial.println("Single press in main");
                stateManager.toggleOutput(i);  // Single press toggle
            }
        }
        
        // Handle double press (triggers on second press down)
        if (currentDoublePress) {
            Serial.println("Double press in main");
            stateManager.resetOutput(i);  // Reset to default state
        }
        
        // Handle long press color cycling
        if (currentLongPress && !prevLongPress[i]) {
            stateManager.setColorCycling(i, true);
        } else if (!currentPress && prevButtonStates[i] && prevLongPress[i]) {
            stateManager.setColorCycling(i, false);
        }
        
        // Update colors if cycling
        stateManager.updateHue(i);
        
        // Store previous states
        prevButtonStates[i] = currentPress;
        prevLongPress[i] = currentLongPress;
    }
    
    // Update physical outputs based on state
    for (int i = 0; i < NUM_BUTTONS; i++) {
        const OutputState& state = stateManager.getState(i);
        updateLED(i, state.isOn, state.hue);
        updateShiftRegister(i + 1, state.isOn);
    }
    
    // Show updates
    showLEDs();
    updateAllShiftRegisters();
    
    delay(50);
}