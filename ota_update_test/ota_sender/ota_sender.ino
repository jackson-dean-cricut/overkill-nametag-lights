#include "painlessMesh.h"
#include <ESP8266WebServer.h>
#include <FS.h>

#define MESH_PREFIX        "whateverYouLike"
#define MESH_PASSWORD      "somethingSneaky"
#define MESH_PORT          5555

#define FIRMWARE_CHUNK_SIZE 1024
#define FIRMWARE_FILENAME   "/firmware.bin"

painlessMesh mesh;
ESP8266WebServer server(80);

// --------------------------------------------------------
// GLOBALS for handling firmware + MD5
// --------------------------------------------------------
size_t    firmwareSize  = 0;
String    targetRole    = "";
bool      firmwareReady = false;

// We'll keep a single open handle for the firmware
File      firmwareFile;  
size_t    totalChunks   = 0;
String    firmwareMD5;  // Store MD5 checksum after upload

// --------------------------------------------------------
// Simple HTML form (no changes)
// --------------------------------------------------------
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

// --------------------------------------------------------
// Handle root page (displays the form)
// --------------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", uploadForm);
}

// --------------------------------------------------------
// Safely close the global firmwareFile if it is open
// --------------------------------------------------------
void safeCloseFile() {
  if (firmwareFile) {
    firmwareFile.close();
  }
}

// --------------------------------------------------------
// newConnectionCallback & changedConnectionCallback
// (no major changes here)
// --------------------------------------------------------
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection: nodeId = %u\n", nodeId);
  Serial.printf("Num nodes: %d\n", mesh.getNodeList().size());
}

void changedConnectionCallback() {
  Serial.println("Changed connections");
  Serial.printf("Num nodes: %d\n", mesh.getNodeList().size());
}

// --------------------------------------------------------
// This is the main file upload handler
//   1) Receives the firmware file via HTTP POST
//   2) Writes it to SPIFFS
//   3) Calculates MD5
//   4) After the upload finishes, we open the firmware
//      file once for reading, initOTASend, and offerOTA.
// --------------------------------------------------------
void handleUpload() {
  HTTPUpload& upload = server.upload();
  static File fsUploadFile;         // local for writing
  static MD5Builder md5;            // local for MD5

  if (upload.status == UPLOAD_FILE_START) {
    firmwareSize  = 0;
    firmwareReady = false;
    safeCloseFile();  // close any leftover open file

    // Get target role from query parameter
    targetRole = server.arg("role");
    Serial.printf("Starting firmware upload for role: %s\n", targetRole.c_str());

    // Make sure the old firmware file is gone
    if (SPIFFS.exists(FIRMWARE_FILENAME)) {
      SPIFFS.remove(FIRMWARE_FILENAME);
    }

    // Open file for writing
    fsUploadFile = SPIFFS.open(FIRMWARE_FILENAME, "w");
    if (!fsUploadFile) {
      Serial.println("Failed to open file for writing");
      return;
    }

    // Prepare MD5
    md5.begin();
  }
  else if (upload.status == UPLOAD_FILE_WRITE && fsUploadFile) {
    // Keep mesh alive during large uploads
    static unsigned long lastMeshUpdate = 0;
    if (millis() - lastMeshUpdate > 100) {
      mesh.update();
      lastMeshUpdate = millis();
    }

    // Write data to SPIFFS
    fsUploadFile.write(upload.buf, upload.currentSize);
    md5.add(upload.buf, upload.currentSize);

    firmwareSize += upload.currentSize;
    Serial.printf("Written %u bytes (Total: %u bytes)\n", 
                  upload.currentSize, firmwareSize);
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();

      // Keep mesh alive and prevent watchdog resets
      mesh.update();
      yield();

      // Finalize MD5
      md5.calculate();
      firmwareMD5 = md5.toString();
      firmwareReady = true;

      Serial.printf("Upload Success: %u bytes\n", firmwareSize);
      Serial.printf("MD5: %s\n", firmwareMD5.c_str());

      // -------------------------------------------------
      // PERSISTENT FILE HANDLING: Open the file once for reading
      // -------------------------------------------------
      firmwareFile = SPIFFS.open(FIRMWARE_FILENAME, "r");
      if (!firmwareFile) {
        Serial.println("Failed to open firmware file for reading!");
        server.send(500, "text/plain", "Failed to open firmware for reading");
        return;
      }

      // Calculate total chunks
      totalChunks = ceil((float)firmwareSize / FIRMWARE_CHUNK_SIZE);

      // -------------------------------------------------
      // Initialize the OTA Send callback (single open file)
      // -------------------------------------------------
      mesh.initOTASend(
        [](painlessmesh::plugin::ota::DataRequest pkg, char* buffer) -> size_t {
          // If file is not open, return 0
          if (!firmwareFile) {
            Serial.println("[OTA Callback] File not open!");
            return 0;
          }

          size_t offset = pkg.partNo * FIRMWARE_CHUNK_SIZE;
          if (offset >= firmwareSize) {
            // No more data
            Serial.printf("[OTA Callback] Offset %u beyond firmware size\n", offset);
            return 0;
          }

          // Seek to the correct position
          firmwareFile.seek(offset);

          // Read up to FIRMWARE_CHUNK_SIZE (or remainder)
          size_t toRead = min((size_t)FIRMWARE_CHUNK_SIZE, firmwareSize - offset);
          size_t readBytes = firmwareFile.read((uint8_t*)buffer, toRead);

          Serial.printf("[OTA Callback] Sending chunk %d/%d (%d bytes)\n",
                        pkg.partNo + 1, totalChunks, readBytes);

          // If this is the last chunk, close the file
          if (pkg.partNo + 1 == totalChunks) {
            Serial.println("[OTA Callback] All chunks sent, closing file...");
            firmwareFile.close();
          }

          return readBytes;
        },
        FIRMWARE_CHUNK_SIZE
      );

      // Small delay for safety
      delay(100);

      // -------------------------------------------------
      // Offer OTA update to nodes with matching role
      // -------------------------------------------------
      Serial.println("\n--- OTA Update Preparation ---");
      Serial.printf("Connected nodes: %d\n", mesh.getNodeList().size());
      Serial.printf("Final firmware size: %u bytes\n", firmwareSize);
      Serial.printf("Number of chunks: %d\n", totalChunks);
      Serial.printf("Target role: %s\n", targetRole.c_str());
      Serial.printf("MD5: %s\n", firmwareMD5.c_str());

      // Keep mesh alive
      mesh.update();
      yield();

      Serial.println("\nOffering OTA update to mesh...");
      if (mesh.getNodeList().size() > 0) {
        auto offerTask = mesh.offerOTA(
          targetRole, 
          "ESP8266",
          firmwareMD5,
          totalChunks,
          false
        );

        if (offerTask) {
          Serial.println("OTA offer task created: Success");
        } else {
          Serial.println("OTA offer task created: Failed");
        }

      } else {
        Serial.println("No nodes in mesh! Cannot offer OTA.");
      }
    }

    // Redirect user back to the upload page
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

// --------------------------------------------------------
// Standard setup()
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS");
    SPIFFS.format();
    if (!SPIFFS.begin()) {
      Serial.println("SPIFFS mount failed after formatting");
      return;
    }
  }

  // Init mesh with debug info
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION | DEBUG | MESH_STATUS);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  // This node is the root
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  // Init web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, []() {
    // This is called when the POST is complete
    server.send(200);
  }, handleUpload);

  server.begin();

  Serial.printf("Sender Node ID: %u\n", mesh.getNodeId());
  Serial.println("Web server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --------------------------------------------------------
// Standard loop()
// --------------------------------------------------------
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
