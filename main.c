#include <Arduino.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// Pin Definitions
#define LED_PIN         5     // Data pin for LED strip
#define SHIFT_DATA_PIN  13    // Data pin for shift register
#define SHIFT_CLOCK_PIN 14    // Clock pin for shift register
#define SHIFT_LATCH_PIN 15    // Latch pin for shift register

const uint8_t BUTTON_PINS[6] = {16, 17, 18, 19, 21, 22}; // Button input pins
const int NUM_LEDS = 60;      // Number of LEDs in strip
const int LEDS_PER_SECTION = NUM_LEDS / 6;  // LEDs controlled by each button

// LED strip array
CRGB leds[NUM_LEDS];

// Button debouncing
bool buttonStates[6] = {false};
bool lastButtonStates[6] = {false};
unsigned long lastDebounceTime[6] = {0};
const unsigned long debounceDelay = 50;

// Device configuration and state
struct DeviceConfig {
    uint8_t macAddress[6];
    bool isHub;
    uint32_t deviceId;
};

struct PatternState {
    uint8_t sectionStates[6];  // On/off state for each section
    uint32_t patternId;
    uint32_t timestamp;
};

DeviceConfig deviceConfig;
PatternState currentState;

// Mesh communication
struct MeshMessage {
    PatternState state;
    bool isUpdate;  // True if this is a firmware update message
    uint8_t updateData[200];  // Space for update chunks if needed
};

// Button handling
void handleButtons() {
    for (int i = 0; i < 6; i++) {
        int reading = digitalRead(BUTTON_PINS[i]);
        if (reading != lastButtonStates[i]) {
            lastDebounceTime[i] = millis();
        }
        
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            if (reading != buttonStates[i]) {
                buttonStates[i] = reading;
                if (buttonStates[i] == LOW) {  // Button pressed
                    currentState.sectionStates[i] = !currentState.sectionStates[i];
                    updateLEDs();
                    if (deviceConfig.isHub) {
                        broadcastState();
                    }
                }
            }
        }
        lastButtonStates[i] = reading;
    }
}

// Shift register control for status LEDs
void updateStatusLEDs() {
    uint8_t statusByte = 0;
    for (int i = 0; i < 6; i++) {
        if (currentState.sectionStates[i]) {
            statusByte |= (1 << i);
        }
    }
    
    digitalWrite(SHIFT_LATCH_PIN, LOW);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, statusByte);
    digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

// LED strip control
void updateLEDs() {
    for (int section = 0; section < 6; section++) {
        int startLed = section * LEDS_PER_SECTION;
        int endLed = startLed + LEDS_PER_SECTION;
        CRGB color = currentState.sectionStates[section] ? CRGB::White : CRGB::Black;
        
        for (int i = startLed; i < endLed; i++) {
            leds[i] = color;
        }
    }
    FastLED.show();
    updateStatusLEDs();
}

// ESP-NOW callbacks
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (len == sizeof(MeshMessage)) {
        MeshMessage* message = (MeshMessage*)data;
        if (!message->isUpdate) {
            memcpy(&currentState, &message->state, sizeof(PatternState));
            updateLEDs();
        } else {
            handleUpdatePacket(message);
        }
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Could add retry logic here if needed
}

// Hub broadcasting
void broadcastState() {
    MeshMessage message;
    message.isUpdate = false;
    memcpy(&message.state, &currentState, sizeof(PatternState));
    
    // Broadcast to all peers
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(broadcastAddress, (uint8_t*)&message, sizeof(MeshMessage));
}

// Role determination
bool isLowestMacInNetwork() {
    // For now, just use this device's MAC
    // In practice, would need to discover other devices
    return true;  // Temporary - first device will be hub
}

// OTA Update setup
void setupOTA() {
    ArduinoOTA.setHostname("LED-Mesh-Node");
    ArduinoOTA.setPassword("mesh-update-pass");
    
    ArduinoOTA.onStart([]() {
        // Turn off all LEDs during update
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });
    
    ArduinoOTA.onEnd([]() {
        // Show update complete
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        FastLED.show();
        delay(500);
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // Show progress on LED strip
        int progressLeds = (progress / (total / NUM_LEDS));
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = i <= progressLeds ? CRGB::Blue : CRGB::Black;
        }
        FastLED.show();
    });
    
    ArduinoOTA.begin();
}

void setup() {
    // Initialize pins
    for (int i = 0; i < 6; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    }
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLOCK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);
    
    // Initialize LED strip
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(64);
    
    // Initialize WiFi for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.macAddress(deviceConfig.macAddress);
    
    // Initialize ESP-NOW
    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb(OnDataRecv);
        esp_now_register_send_cb(OnDataSent);
    }
    
    // Determine device role
    deviceConfig.isHub = isLowestMacInNetwork();
    
    // Setup OTA
    setupOTA();
    
    // Initial LED state
    memset(currentState.sectionStates, 0, sizeof(currentState.sectionStates));
    updateLEDs();
}

void loop() {
    ArduinoOTA.handle();
    handleButtons();
    
    if (deviceConfig.isHub) {
        // Hub-specific tasks could go here
        // Like pattern generation or timing synchronization
    }
}
