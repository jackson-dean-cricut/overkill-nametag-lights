#include <Arduino.h>
#include <Shifty.h>

// Shift register pins
const int dataPin = 16;   // GPIO16 (D0)
const int clockPin = 15;  // GPIO15 (D8)
const int NUM_BITS = 8;   // Total number of bits in shift register

// Create Shifty object
Shifty shift;

void setup() {
  Serial.begin(115200);
  
  // Initialize shift register
  shift.setBitCount(NUM_BITS);
  shift.setPins(dataPin, clockPin, clockPin);  // Using clock as latch
}

void loop() {
  // Simple test pattern: light one LED at a time
  for (int i = 1; i < 7; i++) {
    shift.batchWriteBegin();
    // Clear all bits
    for (int j = 0; j < NUM_BITS; j++) {
      shift.writeBit(j, LOW);
    }
    // Set only the current bit
    shift.writeBit(i, HIGH);
    shift.batchWriteEnd();
    
    Serial.print("Lighting position ");
    Serial.println(i);
    delay(500);  // Wait 500ms between shifts
  }
}