#include "painlessMesh.h"
#include <ESP8266WebServer.h>
#include <FS.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky" 
#define MESH_PORT 5555

#define FIRMWARE_CHUNK_SIZE 1024
#define FIRMWARE_FILENAME "/firmware.bin"

painlessMesh mesh;
ESP8266WebServer server(80);

size_t firmwareSize = 0;
String targetRole = "";
bool firmwareReady = false;
File* currentFile = nullptr;  // Keep track of open file

// HTML form (same as before)
const char* uploadForm = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP8266 OTA Update</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; }
        input[type="file"], input[type="text"] { width: 100%; padding: 8px; }
        input[type="submit"] { 
            background: #4CAF50; 
            color: white; 
            padding: 10px 15px; 
            border: none; 
            cursor: pointer; 
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>ESP8266 Mesh OTA Update</h2>
        <form method="post" action="/upload" enctype="multipart/form-data" id="uploadForm">
            <div class="form-group">
                <label>Firmware Binary (.bin):</label>
                <input type="file" name="firmware" accept=".bin" required>
            </div>
            <div class="form-group">
                <label>Target Role:</label>
                <input type="text" name="role" id="role" required>
            </div>
            <input type="submit" value="Upload & Deploy">
        </form>
        <script>
            document.getElementById('uploadForm').onsubmit = function(e) {
                var role = document.getElementById('role').value;
                this.action = '/upload?role=' + encodeURIComponent(role);
            };
        </script>
    </div>
</body>
</html>
)";

void handleRoot() {
    server.send(200, "text/html", uploadForm);
}

// Function to safely close file if open
void safeCloseFile() {
    if (currentFile != nullptr && currentFile->size() > 0) {
        currentFile->close();
        delete currentFile;
        currentFile = nullptr;
    }
}

void handleUpload() {
    HTTPUpload& upload = server.upload();
    static File fsUploadFile;
    static MD5Builder md5;
    
    if(upload.status == UPLOAD_FILE_START) {
        firmwareSize = 0;
        firmwareReady = false;
        safeCloseFile();  // Ensure any previous file is closed
        
        // Get target role
        targetRole = server.arg("role");
        Serial.printf("Starting firmware upload for role: %s\n", targetRole.c_str());
        
        if(SPIFFS.exists(FIRMWARE_FILENAME)) {
            SPIFFS.remove(FIRMWARE_FILENAME);
        }
        
        fsUploadFile = SPIFFS.open(FIRMWARE_FILENAME, "w");
        if(!fsUploadFile) {
            Serial.println("Failed to open file for writing");
            return;
        }
        
        md5.begin();
    } 
    else if(upload.status == UPLOAD_FILE_WRITE && fsUploadFile) {
        // Keep mesh alive during upload
        static unsigned long lastMeshUpdate = 0;
        if (millis() - lastMeshUpdate > 100) {  // Update mesh every 100ms
            mesh.update();
            lastMeshUpdate = millis();
        }
        
        fsUploadFile.write(upload.buf, upload.currentSize);
        md5.add(upload.buf, upload.currentSize);
        firmwareSize += upload.currentSize;
        Serial.printf("Written %u bytes (Total: %u bytes)\n", upload.currentSize, firmwareSize);
    } 
    else if(upload.status == UPLOAD_FILE_END) {
        if(fsUploadFile) {
            fsUploadFile.close();
            
            // Keep mesh alive and prevent watchdog reset
            mesh.update();
            yield();
            
            md5.calculate();
            firmwareReady = true;
            
            Serial.printf("Upload Success: %u bytes\n", firmwareSize);
            Serial.printf("MD5: %s\n", md5.toString().c_str());
            
            // Initialize OTA sending with safe file handling
            mesh.initOTASend(
                [](painlessmesh::plugin::ota::DataRequest pkg, char* buffer) -> size_t {
                    static File f;
                    size_t read = 0;
                    
                    // Open file for each request
                    f = SPIFFS.open(FIRMWARE_FILENAME, "r");
                    if (!f) {
                        Serial.println("Failed to open firmware file!");
                        return 0;
                    }
                    
                    if (pkg.partNo * FIRMWARE_CHUNK_SIZE < firmwareSize) {
                        f.seek(pkg.partNo * FIRMWARE_CHUNK_SIZE);
                        read = f.read((uint8_t*)buffer, min(
                            static_cast<size_t>(FIRMWARE_CHUNK_SIZE),
                            firmwareSize - (pkg.partNo * FIRMWARE_CHUNK_SIZE)
                        ));
                        int totalChunks = ceil(((float)firmwareSize) / FIRMWARE_CHUNK_SIZE);
                        Serial.printf("Sending chunk %d/%d (%d bytes)\n", pkg.partNo + 1, totalChunks, read);
                    }
                    
                    f.close();
                    return read;
                },
                FIRMWARE_CHUNK_SIZE
            );
            
            // Small delay before offering OTA
            delay(100);
            
            // Prepare for OTA update
            Serial.println("\n--- OTA Update Preparation ---");
            Serial.printf("Connected nodes: %d\n", mesh.getNodeList().size());
            Serial.printf("Final firmware size: %u bytes\n", firmwareSize);
            Serial.printf("Number of chunks: %d\n", (int)ceil(((float)firmwareSize) / FIRMWARE_CHUNK_SIZE));
            Serial.printf("Target role: %s\n", targetRole.c_str());
            Serial.printf("MD5: %s\n", md5.toString().c_str());
            
            // Keep mesh alive
            mesh.update();
            yield();
            
            // Offer OTA update
            Serial.println("\nOffering OTA update to mesh...");
            if (mesh.getNodeList().size() > 0) {
                auto task = mesh.offerOTA(
                    targetRole,
                    "ESP8266",
                    md5.toString(),
                    ceil(((float)firmwareSize) / FIRMWARE_CHUNK_SIZE),
                    false
                );
                Serial.printf("OTA offer task created: %s\n", task ? "Success" : "Failed");
            } else {
                Serial.println("No nodes in mesh! Cannot offer OTA.");
            }
        }
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection: nodeId = %u\n", nodeId);
    Serial.printf("Num nodes: %d\n", mesh.getNodeList().size());
}

void changedConnectionCallback() {
    Serial.println("Changed connections");
    Serial.printf("Num nodes: %d\n", mesh.getNodeList().size());
}

void setup() {
    Serial.begin(115200);
    
    if(!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        SPIFFS.format();
        if(!SPIFFS.begin()) {
            Serial.println("SPIFFS mount failed after formatting");
            return;
        }
    }
    
    // Initialize mesh with full debug output
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION | DEBUG | MESH_STATUS);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, []() {
        server.send(200);
    }, handleUpload);
    
    server.begin();
    
    Serial.printf("Sender Node ID: %u\n", mesh.getNodeId());
    Serial.println("Web server started");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    mesh.update();
    server.handleClient();
    
    // Print status periodically
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) {  // Every 5 seconds
        Serial.printf("Connected nodes: %d\n", mesh.getNodeList().size());
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        if (firmwareSize > 0) {
            Serial.printf("Current firmware size: %u bytes\n", firmwareSize);
        }
        lastStatus = millis();
    }
}