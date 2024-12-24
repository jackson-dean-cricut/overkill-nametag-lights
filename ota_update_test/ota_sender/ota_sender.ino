#include "painlessMesh.h"
#include <ESP8266WebServer.h>
#include <FS.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky" 
#define MESH_PORT 5555

#define FIRMWARE_CHUNK_SIZE 1024  // Size of OTA chunks to send
#define FIRMWARE_FILENAME "/firmware.bin"

painlessMesh mesh;
ESP8266WebServer server(80);

size_t firmwareSize = 0;
String targetRole = "";
bool firmwareReady = false;
MD5Builder md5;

// HTML for the upload form
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
        .status { margin-top: 20px; padding: 10px; border-radius: 4px; }
        .error { background: #ffebee; color: #c62828; }
        .success { background: #e8f5e9; color: #2e7d32; }
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
                // Add the role as a URL parameter
                this.action = '/upload?role=' + encodeURIComponent(role);
            };
        </script>
        <div id="status"></div>
    </div>
</body>
</html>
)";

void handleRoot() {
    server.send(200, "text/html", uploadForm);
}

void handleUpload() {
    HTTPUpload& upload = server.upload();
    static File fsUploadFile;
    
    if(upload.status == UPLOAD_FILE_START) {
        firmwareSize = 0;
        firmwareReady = false;
        
        // Delete existing firmware file if it exists
        if(SPIFFS.exists(FIRMWARE_FILENAME)) {
            SPIFFS.remove(FIRMWARE_FILENAME);
        }
        
        // Open file for writing
        fsUploadFile = SPIFFS.open(FIRMWARE_FILENAME, "w");
        if(!fsUploadFile) {
            Serial.println("Failed to open file for writing");
            return;
        }
        
        // Initialize MD5 builder
        md5.begin();
        
        // Get target role from form data
        if (upload.name == "firmware") {
            // Extract role from the filename field that's sent before the file
            String filename = upload.filename;
            targetRole = server.arg("role");
            Serial.printf("Starting firmware upload for role: %s\n", targetRole.c_str());
        }
    } 
    else if(upload.status == UPLOAD_FILE_WRITE) {
        if(fsUploadFile) {
            // Write chunk to file and update MD5
            fsUploadFile.write(upload.buf, upload.currentSize);
            md5.add(upload.buf, upload.currentSize);
            firmwareSize += upload.currentSize;
            Serial.printf("Uploading: %u\n", firmwareSize);
        }
    } 
    else if(upload.status == UPLOAD_FILE_END) {
        if(fsUploadFile) {
            fsUploadFile.close();
            md5.calculate();
            firmwareReady = true;
            
            Serial.printf("Upload Success: %u bytes\n", upload.totalSize);
            
            // Initialize OTA sending
            mesh.initOTASend(
                [](painlessmesh::plugin::ota::DataRequest pkg, char* buffer) -> size_t {
                    File f = SPIFFS.open(FIRMWARE_FILENAME, "r");
                    if (!f) return 0;
                    
                    if (pkg.partNo * FIRMWARE_CHUNK_SIZE < firmwareSize) {
                        // Seek to the requested chunk
                        f.seek(pkg.partNo * FIRMWARE_CHUNK_SIZE);
                        
                        // Calculate chunk size
                        size_t length = min(
                            static_cast<size_t>(FIRMWARE_CHUNK_SIZE),
                            firmwareSize - (pkg.partNo * FIRMWARE_CHUNK_SIZE)
                        );
                        
                        // Read chunk into buffer
                        size_t read = f.read((uint8_t*)buffer, length);
                        f.close();
                        return read;
                    }
                    
                    f.close();
                    return 0;
                },
                FIRMWARE_CHUNK_SIZE
            );
            
            // Debug print the role and other info
            Serial.println("Preparing OTA offer with:");
            Serial.printf("Role: %s\n", targetRole.c_str());
            Serial.printf("MD5: %s\n", md5.toString().c_str());
            Serial.printf("Firmware size: %u bytes\n", firmwareSize);
            
            // Offer OTA update to mesh
            mesh.offerOTA(
                targetRole,
                "ESP8266",
                md5.toString(),
                ceil(((float)firmwareSize) / FIRMWARE_CHUNK_SIZE),
                false
            );
            
            Serial.printf("OTA update offered to role: %s\n", targetRole.c_str());
        }
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize SPIFFS
    if(!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    
    // Format SPIFFS if mounting failed
    if(!SPIFFS.begin()) {
        Serial.println("SPIFFS formatting...");
        SPIFFS.format();
        if(!SPIFFS.begin()) {
            Serial.println("SPIFFS mount failed after formatting");
            return;
        }
    }
    
    // Initialize mesh network
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    
    // Setup web server
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, []() {
        server.send(200);
    }, handleUpload);
    
    server.begin();
    Serial.println("Web server started");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

int last_print = 0;
void loop() {
    mesh.update();
    server.handleClient();
    int time_now = millis();
    if (time_now - last_print > 5000) {
      last_print = time_now;
      Serial.println(WiFi.localIP());
    }
}