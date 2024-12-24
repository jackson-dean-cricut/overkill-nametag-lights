#include "painlessMesh.h"

// Mesh network credentials - must match your sender
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Built-in LED pin for visual feedback
#ifdef LED_BUILTIN
  #define LED LED_BUILTIN
#else
  #define LED 2  // Most ESP8266 boards have LED on GPIO2
#endif

// Task scheduler
Scheduler userScheduler;
painlessMesh mesh;

// Forward declarations
void sendNodeStatus();
void blinkLED();

// Task to periodically send node status
Task taskSendStatus(TASK_SECOND * 10, TASK_FOREVER, &sendNodeStatus);

// Task to blink LED during updates
Task taskBlink(TASK_MILLISECOND * 250, TASK_FOREVER, &blinkLED);
bool ledState = false;

void blinkLED() {
  ledState = !ledState;
  digitalWrite(LED, ledState);
}

// Sends detailed node status to the network
void sendNodeStatus() {
  String msg = "Node Status: ";
  msg += "\nNode ID: " + String(mesh.getNodeId());
  msg += "\nFree Heap: " + String(ESP.getFreeHeap());
  msg += "\nConnected Nodes: " + String(mesh.getNodeList().size());
  
  // Get and append node time
  uint32_t nodeTime = mesh.getNodeTime();
  msg += "\nNode Time: " + String(nodeTime);
  
  mesh.sendBroadcast(msg);
  Serial.println(msg);
}

// Callback for received messages
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received message from node %u: %s\n", from, msg.c_str());
}

// Callback for new connections
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection: Node ID = %u\n", nodeId);
  Serial.printf("Connection list: %s\n", mesh.subConnectionJson(true).c_str());
}

// Callback for changed connections
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("Node list: ");
  SimpleList<uint32_t> nodes = mesh.getNodeList();
  for (auto node : nodes) {
    Serial.printf(" %u", node);
  }
  Serial.printf("\n");
}

// Callback for node time adjustments
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Time adjusted by %d ms\n", offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  
  // Enable ALL debug message types
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION | SYNC | COMMUNICATION | 
                        GENERAL | MSG_TYPES | REMOTE | MESH_STATUS);

  // Initialize mesh network
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  // Set callbacks
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  // Initialize OTA - this device will accept updates targeted at "debug_receiver" role
  mesh.initOTAReceive("debug_receiver");

  // Add tasks to scheduler
  userScheduler.addTask(taskSendStatus);
  userScheduler.addTask(taskBlink);
  
  // Enable the status sending task
  taskSendStatus.enable();
  
  // Print initial info
  Serial.println("\nOTA Debug Receiver Node Starting");
  Serial.printf("Node ID: %u\n", mesh.getNodeId());
  Serial.printf("WiFi Mode: %s\n", WiFi.getMode() == WIFI_AP ? "AP" : "STA");
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Flash Size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Mesh Prefix: %s\n", MESH_PREFIX);
  Serial.printf("Mesh Port: %d\n", MESH_PORT);
  Serial.printf("OTA Role: debug_receiver\n");
  
  // Flash LED to indicate successful startup
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
  }
}

void loop() {
  mesh.update();
  userScheduler.execute();
}