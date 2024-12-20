#include <Arduino.h>

// Button pins
const int BUTTON_PINS[] = {14, 12, 13, 5, 4, 0};  // Original pins from first sketch
const int NUM_BUTTONS = 6;

void setup() {
  Serial.begin(115200);
  Serial.println("Button test starting...");
  
  // Configure buttons with internal pull-up
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
}

void loop() {
  // Check each button
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Read button state (LOW when pressed due to pull-up)
    bool buttonPressed = !digitalRead(BUTTON_PINS[i]);
    
    if (buttonPressed) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.print(" (GPIO");
      Serial.print(BUTTON_PINS[i]);
      Serial.println(") pressed!");
    }
  }
  
  // Small delay to prevent serial flooding
  delay(100);
}