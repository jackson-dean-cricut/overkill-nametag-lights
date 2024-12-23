#include "button_manager.h"

void ButtonManager::begin() {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].setUserId(i);
        
        buttons[i].setClickHandler([&](EventButton& btn) {
            this->onButtonClicked(btn);
        });
        
        buttons[i].setDoubleClickHandler([&](EventButton& btn) {
            this->onButtonDoubleClicked(btn);
        });
        
        buttons[i].setLongPressHandler([&](EventButton& btn) {
            this->onButtonLongPress(btn);
        });

        buttons[i].setReleasedHandler([&](EventButton& btn) {
            this->onButtonReleased(btn);
        });
        
        buttons[i].setLongClickDuration(500);
    }
}

void ButtonManager::update() {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].update();
    }
}

void ButtonManager::onButtonClicked(EventButton& btn) {
    ButtonEventData event{ButtonEvent::CLICKED, btn.userId()};
    EventBus::publish(event);
}

void ButtonManager::onButtonDoubleClicked(EventButton& btn) {
    ButtonEventData event{ButtonEvent::DOUBLE_CLICKED, btn.userId()};
    EventBus::publish(event);
}

void ButtonManager::onButtonLongPress(EventButton& btn) {
    ButtonEventData event{ButtonEvent::LONG_PRESSED, btn.userId()};
    EventBus::publish(event);
}

void ButtonManager::onButtonReleased(EventButton& btn) {
    // Only send release event if it was a long press
    if (btn.previousDuration() > 500) {
        ButtonEventData event{ButtonEvent::LONG_PRESS_RELEASED, btn.userId()};
        EventBus::publish(event);
    }
}