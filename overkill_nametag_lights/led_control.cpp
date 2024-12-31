#include "led_control.h"

// Define the sine lookup table
const uint8_t LEDUtils::_sin8Table[] PROGMEM = {
    128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
    176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
    218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,
    245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
    255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
    245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,
    218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
    176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
    128,124,121,118,115,112,109,106,103,100,97,93,90,88,85,82,
    79,76,73,70,67,65,62,59,57,54,52,49,47,44,42,40,
    37,35,33,31,29,27,25,23,21,20,18,17,15,14,12,11,
    10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,
    0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,
    10,11,12,14,15,17,18,20,21,23,25,27,29,31,33,35,
    37,40,42,44,47,49,52,54,57,59,62,65,67,70,73,76,
    79,82,85,88,90,93,97,100,103,106,109,112,115,118,121,124
};

LEDController::LEDController() : 
    strip(NUM_LEDS, LED_PIN)
{
    memset(ledHues, 0, sizeof(ledHues));
    memset(ledBrightness, 255, sizeof(ledBrightness));
    memset(ledActive, 0, sizeof(ledActive));
    randomSeed(analogRead(0)); // Initialize random number generator
}

void LEDController::begin() {
    strip.Begin();
    strip.Show(); // Clear all pixels
}

void LEDController::updateLED(int index, bool isOn, uint8_t hue, uint8_t brightness) {
    if (index >= NUM_LEDS) return;
    
    ledActive[index] = isOn;
    ledHues[index] = hue;
    ledBrightness[index] = brightness;
    
    if (isOn) {
        RgbColor color = hsvToRgb(hue, 255, brightness);
        color = LEDUtils::applyGamma(color);
        strip.SetPixelColor(index, color);
    } else {
        strip.SetPixelColor(index, RgbColor(0));
    }
}

void LEDController::show() {
    strip.Show();
}

RgbColor LEDController::hsvToRgb(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r, g, b;
    
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    
    return RgbColor(r, g, b);
}