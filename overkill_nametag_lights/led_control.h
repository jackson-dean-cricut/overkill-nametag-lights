#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <NeoPixelBus.h>

// If defined, use the table-based gamma correction, using more memory but is faster
// If not defined, use the equation based correction. Slower but smaller.
#define USE_GAMMA_TABLE 

// Utility class to replace FastLED math functions
class LEDUtils {
public:
    static uint8_t sin8(uint8_t x) {
        return scale8(pgm_read_byte(&_sin8Table[x]), 255);
    }
    
    static uint8_t random8() {
        return random(256);
    }
    
    static uint8_t random8(uint8_t max) {
        return random(max);
    }
    
    static uint8_t random8(uint8_t min, uint8_t max) {
        return random(min, max);
    }
    
    static uint8_t scale8(uint8_t i, uint8_t scale) {
        return ((uint16_t)i * (uint16_t)scale) >> 8;
    }

    static RgbColor applyGamma(RgbColor color) {
#ifdef USE_GAMMA_TABLE
        static NeoGamma<NeoGammaTableMethod> colorGamma;
#else
        static NeoGamma<NeoGammaEquationMethod> colorGamma;
#endif
        return colorGamma.Correct(color);
    }

private:
    // Pre-calculated sine table
    static const uint8_t _sin8Table[] PROGMEM;
};

class LEDController {
public:
    static const uint16_t NUM_LEDS = 6;
    static const uint8_t LED_PIN = 2;
    static const uint8_t BRIGHTNESS = 128;

    LEDController();
    void begin();
    void updateLED(int index, bool isOn, uint8_t hue, uint8_t brightness);
    void show();

private:
    // Using NeoPixelBus with Neo800KbpsMethod for WS2811
    NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1Ws2812xMethod> strip;
    uint8_t ledHues[NUM_LEDS];
    uint8_t ledBrightness[NUM_LEDS];
    bool ledActive[NUM_LEDS];
    
    RgbColor hsvToRgb(uint8_t h, uint8_t s, uint8_t v);
};

#endif