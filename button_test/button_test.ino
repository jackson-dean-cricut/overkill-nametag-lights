#include <EventButton.h>

class ButtonManager {
public:
    static const int NUM_BUTTONS = 6;
    const int BUTTON_PINS[NUM_BUTTONS] = {14, 12, 13, 5, 4, 0};
    
    // Initialize the buttons in the constructor initialization list
    ButtonManager() : 
        buttons{
            EventButton(BUTTON_PINS[0]),
            EventButton(BUTTON_PINS[1]),
            EventButton(BUTTON_PINS[2]),
            EventButton(BUTTON_PINS[3]),
            EventButton(BUTTON_PINS[4]),
            EventButton(BUTTON_PINS[5])
        } 
    {}
    
    void begin() {
        // Configure all buttons
        for(int i = 0; i < NUM_BUTTONS; i++) {
            buttons[i].setUserId(i);
            
            // Set up handlers using lambda functions to call class methods
            buttons[i].setClickHandler([&](EventButton& btn) {
                this->onButtonClicked(btn);
            });
            
            buttons[i].setDoubleClickHandler([&](EventButton& btn) {
                this->onButtonDoubleClicked(btn);
            });
            
            buttons[i].setLongPressHandler([&](EventButton& btn) {
                this->onButtonLongPress(btn);
            });
            
            buttons[i].setLongClickDuration(500);
        }
    }
    
    void update() {
        for(int i = 0; i < NUM_BUTTONS; i++) {
            buttons[i].update();
        }
    }

private:
    EventButton buttons[NUM_BUTTONS];
    
    void onButtonClicked(EventButton& btn) {
        int buttonIndex = btn.userId();
        Serial.printf("Button %d clicked\n", buttonIndex);
    }
    
    void onButtonDoubleClicked(EventButton& btn) {
        int buttonIndex = btn.userId();
        Serial.printf("Button %d double clicked\n", buttonIndex);
    }
    
    void onButtonLongPress(EventButton& btn) {
        int buttonIndex = btn.userId();
        Serial.printf("Button %d long pressed\n", buttonIndex);
    }
};

// Global instance
ButtonManager buttonManager;

void setup() {
    Serial.begin(115200);
    buttonManager.begin();
}

void loop() {
    buttonManager.update();
    delay(20);
}