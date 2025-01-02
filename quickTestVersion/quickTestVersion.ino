#include <Arduino.h>
#include <FastLED.h>

// LED strip configuration
#define LED_PIN     2      // ESP8266 GPIO2
#define NUM_LEDS    6
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS  32    // Set to about 50%

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Clear all LEDs to start
  FastLED.clear();
  FastLED.show();
  
  // Initial delay to allow programming if needed
  delay(2000);
}

void loop() {
  // Create a rainbow effect across all LEDs
  static uint8_t hue = 0;
  
  // Fill the LED array with a rainbow
  fill_rainbow(leds, NUM_LEDS, hue, 255/NUM_LEDS);
  
  // Show the LEDs
  FastLED.show();
  
  // Slowly cycle through colors
  hue++;
  
  // Small delay to control animation speed
  delay(50);
}