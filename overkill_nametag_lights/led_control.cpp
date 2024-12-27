#include "led_control.h"

LEDController::LEDController() {
    memset(ledHues, 0, sizeof(ledHues));
    memset(ledActive, 0, sizeof(ledActive));
}

void LEDController::begin() {
    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    delay(1); // allow system to stabilize
    FastLED.setMaxRefreshRate(400); // limit updates to 400Hz
    FastLED.clear();
    FastLED.show();
}

void LEDController::updateLED(int index, bool isOn, uint8_t hue) {
    if (index >= NUM_LEDS) return;
    ledActive[index] = isOn;
    
    if (isOn) {
        leds[index] = CHSV(hue, 255, 255);
    } else {
        leds[index] = CRGB::Black;
    }
}

void LEDController::show() {
    FastLED.show();
}