#include "shift_register.h"

Shifty shiftRegister;
bool shiftStates[NUM_OUTPUTS] = {false};

void setupShiftRegister() {
  shiftRegister.setBitCount(NUM_OUTPUTS);
  shiftRegister.setPins(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, SHIFT_CLOCK_PIN);
}

void updateShiftRegister(int index, bool state) {
  if (index >= NUM_OUTPUTS) return;
  shiftStates[index] = state;
}

void updateAllShiftRegisters() {
  shiftRegister.batchWriteBegin();
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    shiftRegister.writeBit(i, shiftStates[i]);
  }
  shiftRegister.batchWriteEnd();
}