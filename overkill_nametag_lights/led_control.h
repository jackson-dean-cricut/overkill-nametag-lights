#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <FastLED.h>

class LEDController {
public:
    static const int NUM_LEDS = 6;
    static const int LED_PIN = 2;
    static const int BRIGHTNESS = 128;

    LEDController();
    void begin();
    void updateLED(int index, bool isOn, uint8_t hue);
    void show();

private:
    CRGB leds[NUM_LEDS];
    uint8_t ledHues[NUM_LEDS];
    bool ledActive[NUM_LEDS];
};

#endif
