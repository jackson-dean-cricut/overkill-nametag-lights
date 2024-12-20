#include "output_manager.h"
#include "buttons.h"
#include "led_control.h"
#include "shift_register.h"

void updateOutputs() {
  // Update both LED strip and shift register based on button states
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool isPressed = isButtonPressed(i);
    bool isLongPressed = isLongPress(i);
    
    // Update WS2811 LED strip
    updateLED(i, isPressed, isLongPressed);
    
    // Update shift register LED
    updateShiftRegister(i + 1, isPressed);  // +2 offset as per working shift register code
  }
  
  // Show all updates
  showLEDs();
  updateAllShiftRegisters();
}