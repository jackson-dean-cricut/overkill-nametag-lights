#include <Arduino.h>
#include <FastLED.h>
#include <Shifty.h>

// Button pins
const int BUTTON_PINS[] = {14, 12, 13, 5, 4, 0};
const int NUM_BUTTONS = 6;

// Shift register pins
const int SHIFT_DATA_PIN = 16;
const int SHIFT_CLOCK_PIN = 15;

// WS2811 LED strip configuration
#define WS2811_PIN 2  // Choose an appropriate data pin
#define NUM_LEDS 6

// State tracking
bool buttonStates[NUM_BUTTONS] = {false};
bool ledStates[NUM_BUTTONS] = {false};
unsigned long buttonPressTime[NUM_BUTTONS] = {0};
const unsigned long LONG_PRESS_TIME = 500; // 500ms for color cycling

// Color tracking with persistence
uint8_t currentHue[NUM_BUTTONS] = {0};
bool colorSet[NUM_BUTTONS] = {false};

// Objects
CRGB leds[NUM_LEDS];
Shifty shift;

void setup() {
  Serial.begin(115200);

  // Configure buttons with internal pull-up
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  // Shift register setup
  shift.setBitCount(NUM_BUTTONS);
  shift.setPins(SHIFT_DATA_PIN,SHIFT_CLOCK_PIN,SHIFT_CLOCK_PIN);

  // FastLED setup
  FastLED.addLeds<WS2811, WS2811_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);  // Adjust brightness as needed
}

void updateLEDs() {
  // Update shift register LEDs
  shift.batchWriteBegin();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    shift.writeBit(i, ledStates[i]);
  }
  shift.batchWriteEnd();

  // Update WS2811 LEDs
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (ledStates[i]) {
      if (!colorSet[i]) {
        // Default to white if no color set
        leds[i] = CRGB::White;
      } else {
        // Use the stored hue
        leds[i] = CHSV(currentHue[i], 255, 255);
      }
    } else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}

void handleButtonPress(int buttonIndex) {
  // Toggle LED state
  ledStates[buttonIndex] = !ledStates[buttonIndex];
  
  // If turning off, keep color state
  // If turning on without a color, it will default to white
  
  updateLEDs();
}

void handleLongPress(int buttonIndex) {
  // Cycle through hues
  if (!colorSet[buttonIndex]) {
    // First long press starts color cycling
    colorSet[buttonIndex] = true;
    currentHue[buttonIndex] = 0;
  } else {
    // Increment hue by 32 (gives good color spread)
    currentHue[buttonIndex] += 32;
    
    // Wrap back to white/reset if we've gone through all hues
    if (currentHue[buttonIndex] >= 255) {
      colorSet[buttonIndex] = false;
    }
  }
  
  // Ensure LED is on when cycling colors
  ledStates[buttonIndex] = true;
  
  updateLEDs();
}

void loop() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool currentButtonState = !digitalRead(BUTTON_PINS[i]);

    if (currentButtonState && !buttonStates[i]) {
      // Button just pressed
      buttonPressTime[i] = millis();
    }
    
    if (currentButtonState && buttonStates[i]) {
      // Button held down
      if (millis() - buttonPressTime[i] > LONG_PRESS_TIME) {
        handleLongPress(i);
        buttonPressTime[i] = millis(); // Prevent rapid color cycling
      }
    }
    
    if (!currentButtonState && buttonStates[i]) {
      // Button released after short press
      if (millis() - buttonPressTime[i] < LONG_PRESS_TIME) {
        handleButtonPress(i);
      }
    }
    
    buttonStates[i] = currentButtonState;
  }

  delay(50);  // Simple debounce delay
}
