#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "button_manager.h"
#include "led_control.h"
#include "shift_register.h"
#include "state_manager.h"
#include "output_manager.h"

#define DEBUG_MODE
#ifdef DEBUG_MODE
    #define DEBUG_PRINT(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
#endif

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

String ap_ssid;      // Name for the WiFi network
const char* ap_password = "12345678";     // Password for the WiFi network

ButtonManager buttonManager;
StateManager stateManager;
OutputManager outputManager;

String getUniqueSSID() {
    uint32_t chipId = ESP.getChipId();
    char ssid[32];
    snprintf(ssid, sizeof(ssid), "Nametag_%06X", chipId);  // Format chip ID as 6-digit hex
    return String(ssid);
}

void setupOTA() {
    // Configure access point
    ap_ssid = getUniqueSSID();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    
    DEBUG_PRINT("Access Point Started");
    DEBUG_PRINT("SSID: " + String(ap_ssid));
    DEBUG_PRINT("IP address: " + WiFi.softAPIP().toString());

    // Configure OTA
    ArduinoOTA.setPassword("admin");      // OTA password
    
    ArduinoOTA.onStart([]() {
        DEBUG_PRINT("Starting OTA Update");
    });
    
    ArduinoOTA.onEnd([]() {
        DEBUG_PRINT("\nOTA Update Complete");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUG_PRINT("Progress: " + String((progress / (total / 100))) + "%");
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINT("Error[" + String(error) + "]: ");
        if (error == OTA_AUTH_ERROR) DEBUG_PRINT("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) DEBUG_PRINT("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) DEBUG_PRINT("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINT("Receive Failed");
        else if (error == OTA_END_ERROR) DEBUG_PRINT("End Failed");
    });
    
    ArduinoOTA.begin();
    DEBUG_PRINT("OTA Ready");
}

void setup() {
    Serial.begin(115200);
    DEBUG_PRINT("Starting up...");
    
    setupOTA();
    
    buttonManager.begin();
    outputManager.begin(stateManager);
    
    // Subscribe StateManager to button events
    EventBus::subscribe([](const ButtonEventData& event) {
        switch(event.type) {
            case ButtonEvent::CLICKED:
                if (!stateManager.isInAnimationMode()) {
                    stateManager.toggleOutput(event.buttonIndex);
                    DEBUG_PRINT("Toggling output");
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
    ArduinoOTA.handle();  // Handle OTA update requests
    
    buttonManager.update();
    stateManager.update();
    outputManager.update();
    
    delay(20);
}