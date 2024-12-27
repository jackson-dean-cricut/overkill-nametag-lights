// mesh_test.ino

#include <painlessMesh.h>
// #include "messages.h"  // Our shared library
#include "../libraries/NametagShared/src/messages.h"

// Mesh network settings
#define   MESH_PREFIX     "nametagMesh"
#define   MESH_PASSWORD   "testPassword123"
#define   MESH_PORT       5555

// GPIO pins - adjust as needed for your ESP8266
#define   BUTTON_PIN      14  // Use built-in FLASH button on NodeMCU
#define   LED_PIN         LED_BUILTIN  // Built-in LED on most NodeMCU boards

// Globals
painlessMesh mesh;
bool isBridge = false;  // Set this true for bridge node, false for clients
unsigned long lastDebounceTime = 0;
const long debounceDelay = 250;  // Debounce time in ms

// Forward declarations
void sendMessage();
void receivedCallback(uint32_t from, String &msg);

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED off (usually inverted on NodeMCU)

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive(&receivedCallback);

    if (isBridge) {
        mesh.setRoot(true);
        mesh.setContainsRoot(true);
        Serial.println("Bridge node started");
    } else {
        Serial.println("Client node started");
    }
}

void loop() {
    mesh.update();

    // Handle button press with debounce
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (buttonState == LOW) {  // Button pressed
            sendMessage();
        }
    }
    
    lastButtonState = buttonState;
}

void sendMessage() {
    StaticJsonDocument<200> doc;
    
    if (isBridge) {
        // Bridge sends a command
        nametag::Command cmd;
        cmd.type = nametag::Command::Type::SET_ANIMATION;
        cmd.params["pattern"] = random(0, 6);  // Random animation pattern
        
        nametag::createMessagePacket(doc, nametag::MessageType::COMMAND, cmd);
    } else {
        // Client sends an event
        nametag::ClientEvent event;
        event.buttonIndex = 0;
        event.action = nametag::ClientEvent::Action::CLICKED;
        
        nametag::createMessagePacket(doc, nametag::MessageType::CLIENT_EVENT, event);
    }
    
    String msg;
    serializeJson(doc, msg);
    
    if (isBridge) {
        mesh.sendBroadcast(msg);
        Serial.println("Bridge sent command: " + msg);
    } else {
        // Clients send to root node
        auto nodes = mesh.getNodeList(true);
        if (nodes.size() > 0) {
            mesh.sendSingle(nodes.front(), msg);
            Serial.println("Client sent event: " + msg);
        }
    }
}

void receivedCallback(uint32_t from, String &msg) {
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, msg);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
    }
    
    // Flash LED to show message received
    digitalWrite(LED_PIN, LOW);  // LED on
    delay(100);
    digitalWrite(LED_PIN, HIGH);  // LED off
    
    // Handle message based on type
    int messageType = doc["type"] | -1;
    
    switch (static_cast<nametag::MessageType>(messageType)) {
        case nametag::MessageType::COMMAND:
            if (!isBridge) {
                nametag::Command cmd;
                cmd.fromJson(doc["data"]);
                Serial.printf("Received command type: %d\n", static_cast<int>(cmd.type));
            }
            break;
            
        case nametag::MessageType::CLIENT_EVENT:
            if (isBridge) {
                nametag::ClientEvent event;
                event.fromJson(doc["data"]);
                Serial.printf("Received event from button: %d\n", event.buttonIndex);
            }
            break;
            
        default:
            Serial.println("Unknown message type");
            break;
    }
}