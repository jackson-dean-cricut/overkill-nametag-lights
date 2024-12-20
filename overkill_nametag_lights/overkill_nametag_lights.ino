#include <Arduino.h>
#include "button_manager.h"
#include "led_control.h"
#include "shift_register.h"
#include "state_manager.h"
#include "output_manager.h"

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

ButtonManager buttonManager;
StateManager stateManager;

OutputManager outputManager;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");
    
    buttonManager.begin();
    outputManager.begin(stateManager);
    
    // Subscribe StateManager to button events
    EventBus::subscribe([](const ButtonEventData& event) {
        switch(event.type) {
            case ButtonEvent::CLICKED:
                if (!stateManager.isInAnimationMode()) {
                    stateManager.toggleOutput(event.buttonIndex);
                } else {
                    stateManager.setAnimationPattern(event.buttonIndex);
                }
                break;
                
            case ButtonEvent::DOUBLE_CLICKED:
                stateManager.toggleAnimationMode();
                break;
                
            case ButtonEvent::LONG_PRESSED:
                if (!stateManager.isInAnimationMode()) {
                    stateManager.setColorCycling(event.buttonIndex, true);
                }
                break;
                
            case ButtonEvent::LONG_PRESS_RELEASED:
                if (!stateManager.isInAnimationMode()) {
                    stateManager.setColorCycling(event.buttonIndex, false);
                }
                break;
        }
    });
    
    delay(2000);  // Initial delay for programming
}

void loop() {
    buttonManager.update();
    
    stateManager.update();
    
    // Update physical outputs
    outputManager.update();
    
    delay(20);
}