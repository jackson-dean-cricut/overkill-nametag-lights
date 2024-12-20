#include "led_control.h"

CRGB leds[NUM_LEDS];
uint8_t ledHues[NUM_LEDS] = {0};
bool ledActive[NUM_LEDS] = {false};

void setupLEDs() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

void updateLED(int index, bool isOn, uint8_t hue) {
  if (index >= NUM_LEDS) return;
  
  ledActive[index] = isOn;
  
  if (isOn) {
    leds[index] = CHSV(hue, 255, 255);
  } else {
    leds[index] = CRGB::Black;
  }
}

void showLEDs() {
  FastLED.show();
}