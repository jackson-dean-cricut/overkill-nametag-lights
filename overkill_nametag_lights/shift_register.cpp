#include "shift_register.h"

ShiftRegisterController::ShiftRegisterController() {
    memset(shiftStates, 0, sizeof(shiftStates));
}

void ShiftRegisterController::begin() {
    shiftRegister.setBitCount(NUM_OUTPUTS);
    shiftRegister.setPins(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, SHIFT_CLOCK_PIN);
}

void ShiftRegisterController::updateRegister(int index, bool state) {
    if (index >= NUM_OUTPUTS) return;
    shiftStates[index] = state;
}

void ShiftRegisterController::updateAll() {
    shiftRegister.batchWriteBegin();
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        shiftRegister.writeBit(i, shiftStates[i]);
    }
    shiftRegister.batchWriteEnd();
}