#include <Arduino.h>
#include "buttons.h"
#include "led_control.h"
#include "shift_register.h"
#include "output_manager.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");
  
  setupButtons();
  setupLEDs();
  setupShiftRegister();
  
  // Initial delay to allow programming if needed
  delay(2000);
}

void loop() {
  // Handle button inputs
  updateButtons();
  
  // Update visual outputs based on button states
  updateOutputs();
  
  // Small delay for stability
  delay(50);
}