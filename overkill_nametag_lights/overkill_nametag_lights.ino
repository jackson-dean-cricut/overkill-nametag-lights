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
    
    // Check for animation mode toggle
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool currentPress = isButtonPressed(i);
        bool currentLongPress = isLongPress(i);
        bool currentDoublePress = isDoublePress(i);
        
        if (currentDoublePress) {
            if (!stateManager.isInAnimationMode()) {
                // Enter animation mode
                stateManager.toggleAnimationMode();
            } else {
                // Exit animation mode
                stateManager.toggleAnimationMode();
            }
        } else if (stateManager.isInAnimationMode() && currentPress && !prevButtonStates[i]) {
            // In animation mode, single press changes pattern
            stateManager.setAnimationPattern(i);
        } else if (!stateManager.isInAnimationMode()) {
            // Normal mode behavior
            if (!currentPress && prevButtonStates[i]) {
                if (!prevLongPress[i]) {
                    stateManager.toggleOutput(i);
                }
            }
            
            if (currentLongPress && !prevLongPress[i]) {
                stateManager.setColorCycling(i, true);
            } else if (!currentPress && prevButtonStates[i] && prevLongPress[i]) {
                stateManager.setColorCycling(i, false);
            }
            
            stateManager.updateHue(i);
        }
        
        prevButtonStates[i] = currentPress;
        prevLongPress[i] = currentLongPress;
    }
    
    // Update animations if in animation mode
    if (stateManager.isInAnimationMode()) {
        stateManager.updateAnimations();
    }
    
    // Update physical outputs
    for (int i = 0; i < NUM_BUTTONS; i++) {
        const OutputState& state = stateManager.getState(i);
        if (stateManager.isInAnimationMode()) {
            // WS2811 LEDs show the animation
            updateLED(i, state.isOn, state.hue);
            // Shift register LEDs show which animation is active
            updateShiftRegister(i + 1, (i == stateManager.getAnimationPattern()));
        } else {
            // Normal mode - both LED types show button state
            updateLED(i, state.isOn, state.hue);
            updateShiftRegister(i + 1, state.isOn);
        }
    }
    
    showLEDs();
    updateAllShiftRegisters();
    
    delay(20);  // Slightly faster update rate for smoother animations
}