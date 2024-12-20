#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <FastLED.h>

// LED strip configuration
#define LED_PIN     2
#define NUM_LEDS    6
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS  128

extern CRGB leds[NUM_LEDS];
extern uint8_t ledHues[NUM_LEDS];
extern bool ledActive[NUM_LEDS];

void setupLEDs();
void updateLED(int index, bool isOn, bool cycleColor = false);
void showLEDs();

#endif