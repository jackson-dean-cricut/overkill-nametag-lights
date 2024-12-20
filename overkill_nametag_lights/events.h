#ifndef EVENTS_H
#define EVENTS_H

#include <vector>
#include <functional>

// Define event types
enum class ButtonEvent {
    CLICKED,
    DOUBLE_CLICKED,
    LONG_PRESSED,
    LONG_PRESS_RELEASED  // Added to handle color cycling end
};

// Event structure
struct ButtonEventData {
    ButtonEvent type;
    int buttonIndex;
};

// Event handler function type
using EventHandler = std::function<void(const ButtonEventData&)>;

// Event bus
class EventBus {
public:
    static void subscribe(EventHandler handler) {
        handlers.push_back(handler);
    }
    
    static void publish(const ButtonEventData& event) {
        for(auto& handler : handlers) {
            handler(event);
        }
    }

private:
    static std::vector<EventHandler> handlers;
};

#endif