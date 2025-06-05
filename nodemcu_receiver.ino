/*
  NodeMCU Serial Data Receiver
  
  Menerima data sensor dari Arduino via UART dan menampilkan
  di Serial Monitor untuk debugging dan monitoring
  
  Hardware:
  - NodeMCU ESP8266 (3.3V)
  - Level Converter (3.3V ↔ 5V)
  - Built-in LED for status indication
  
  Connections:
  - D2 (GPIO4) NodeMCU (RX) <- Level Converter LV1 <- Arduino Pin 2 (TX)
  - D1 (GPIO5) NodeMCU (TX) -> Level Converter LV2 -> Arduino Pin 3 (RX)
  - USB Serial for debugging and commands
  
  Features:
  - Data parsing dan validasi
  - Serial Monitor logging dengan format tabel
  - LED status indicator
  - Error handling dan statistics
  - Interactive commands via Serial Monitor
  - Communication timeout detection
  
  Author: Your Name
  Date: 2025
  Version: 1.0
*/

#include <SoftwareSerial.h>

// ===== PIN DEFINITIONS =====
#define NODEMCU_RX_PIN D2     // GPIO4 - menerima dari Arduino TX
#define NODEMCU_TX_PIN D1     // GPIO5 - mengirim ke Arduino RX
#define LED_BUILTIN_PIN 2     // Built-in LED (inverted logic)

// ===== SERIAL COMMUNICATION =====
SoftwareSerial arduinoSerial(NODEMCU_RX_PIN, NODEMCU_TX_PIN);

// ===== DATA STRUCTURES =====
struct SensorData {
  float temperature;
  int lightLevel;
  unsigned long timestamp;
  unsigned long receivedTime;
  int packetNumber;
  String arduinoID;
  bool dataValid;
  int dataCount;
} sensorData;

struct SystemStatus {
  bool arduinoConnected;
  unsigned long lastDataReceived;
  unsigned long uptime;
  int totalPacketsReceived;
  int errorCount;
  String lastError;
  unsigned long startTime;
} systemStatus;

// ===== TIMING VARIABLES =====
unsigned long lastHeartbeat = 0;
unsigned long lastDataCheck = 0;
unsigned long lastStatsDisplay = 0;

const unsigned long HEARTBEAT_INTERVAL = 30000;    // 30 detik
const unsigned long DATA_TIMEOUT = 10000;          // 10 detik tanpa data = disconnect
const unsigned long STATS_INTERVAL = 15000;        // Display stats setiap 15 detik

// ===== SETUP FUNCTION =====
void setup() {
  // Inisialisasi Serial untuk debugging
  Serial.begin(9600);
  Serial.println();
  Serial.println("=====================================");
  Serial.println("     NodeMCU Serial Data Receiver");
  Serial.println("=====================================");
  
  // Setup pins
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH); // LED mati (inverted)
  
  // Inisialisasi komunikasi dengan Arduino
  arduinoSerial.begin(9600);
  Serial.println("SoftwareSerial initialized pada D1, D2");
  
  // Inisialisasi data structures
  initializeData();
  
  Serial.println("=====================================");
  Serial.println("System ready! Waiting for Arduino data...");
  Serial.println("Commands: Type 'stats' for statistics");
  Serial.println("          Type 'reset' to reset counters");
  Serial.println("          Type 'help' for all commands");
  Serial.println("=====================================");
  
  // LED indicator setup complete
  blinkLED(3, 200);
}

// ===== MAIN LOOP =====
void loop() {
  // Read data from Arduino
  handleArduinoData();
  
  // Check Arduino connection status
  if (millis() - lastDataCheck >= DATA_TIMEOUT) {
    checkArduinoConnection();
    lastDataCheck = millis();
  }
  
  // Send heartbeat to Arduino
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
  
  // Display periodic statistics
  if (millis() - lastStatsDisplay >= STATS_INTERVAL) {
    displayStatistics();
    lastStatsDisplay = millis();
  }
  
  // Handle Serial Monitor commands
  handleSerialCommands();
  
  // Update system status
  updateSystemStatus();
  
  // Update status LED
  updateStatusLED();
  
  delay(10);
}

// ===== INITIALIZATION =====
void initializeData() {
  sensorData.temperature = 0.0;
  sensorData.lightLevel = 0;
  sensorData.timestamp = 0;
  sensorData.receivedTime = 0;
  sensorData.packetNumber = 0;
  sensorData.arduinoID = "";
  sensorData.dataValid = false;
  sensorData.dataCount = 0;
  
  systemStatus.arduinoConnected = false;
  systemStatus.lastDataReceived = 0;
  systemStatus.startTime = millis();
  systemStatus.uptime = 0;
  systemStatus.totalPacketsReceived = 0;
  systemStatus.errorCount = 0;
  systemStatus.lastError = "";
}

// ===== ARDUINO COMMUNICATION =====
void handleArduinoData() {
  if (arduinoSerial.available()) {
    String receivedData = arduinoSerial.readStringUntil(';');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
      Serial.println();
      Serial.println("<<< RECEIVED: " + receivedData);
      systemStatus.lastDataReceived = millis();
      systemStatus.totalPacketsReceived++;
      
      // Parse dan proses data
      if (parseData(receivedData)) {
        // Kirim konfirmasi ke Arduino
        arduinoSerial.println("OK");
        Serial.println(">>> SENT: OK");
        
        // Update connection status
        systemStatus.arduinoConnected = true;
        
        // LED indicator
        blinkLED(1, 50);
        
        // Display parsed data
        printSensorData();
        
      } else {
        // Error parsing
        arduinoSerial.println("ERROR:PARSE_FAILED");
        Serial.println(">>> SENT: ERROR:PARSE_FAILED");
        systemStatus.errorCount++;
        systemStatus.lastError = "Parse failed for: " + receivedData;
        blinkLED(3, 100);
      }
    }
  }
}

bool parseData(String data) {
  // Handle different data types
  if (data.startsWith("TEMP:")) {
    return parseSensorData(data);
  } else if (data.startsWith("HEARTBEAT:")) {
    return parseHeartbeat(data);
  } else if (data.startsWith("TEST:")) {
    return parseTestData(data);
  } else if (data.startsWith("CUSTOM:")) {
    return parseCustomData(data);
  }
  
  // Return false if no matching data type found
  Serial.println("✗ Unknown data format: " + data);
  return false;
}

bool parseSensorData(String data) {
  // Format: "TEMP:25.30,LIGHT:512,TIME:123456,PACKET:001,ID:ARDUINO01"
  
  int tempIndex = data.indexOf("TEMP:");
  int lightIndex = data.indexOf("LIGHT:");
  int timeIndex = data.indexOf("TIME:");
  int packetIndex = data.indexOf("PACKET:");
  int idIndex = data.indexOf("ID:");
  
  if (tempIndex == -1 || lightIndex == -1 || timeIndex == -1) {
    return false;
  }
  
  // Parse temperature
  int tempEnd = data.indexOf(",", tempIndex);
  if (tempEnd == -1) return false;
  String tempStr = data.substring(tempIndex + 5, tempEnd);
  float newTemp = tempStr.toFloat();
  
  // Parse light level
  int lightEnd = data.indexOf(",", lightIndex);
  if (lightEnd == -1) return false;
  String lightStr = data.substring(lightIndex + 6, lightEnd);
  int newLight = lightStr.toInt();
  
  // Parse timestamp
  int timeEnd = data.indexOf(",", timeIndex);
  if (timeEnd == -1) timeEnd = data.length();
  String timeStr = data.substring(timeIndex + 5, timeEnd);
  unsigned long newTime = timeStr.toInt();
  
  // Parse packet number (optional)
  int newPacket = 0;
  if (packetIndex != -1) {
    int packetEnd = data.indexOf(",", packetIndex);
    if (packetEnd == -1) packetEnd = data.length();
    String packetStr = data.substring(packetIndex + 8, packetEnd);
    newPacket = packetStr.toInt();
  }
  
  // Parse Arduino ID (optional)
  String newID = "";
  if (idIndex != -1) {
    newID = data.substring(idIndex + 3);
    newID.replace(",", "");
  }
  
  // Validate data ranges
  if (newTemp < -50 || newTemp > 150) {
    Serial.println("⚠ Temperature out of range: " + String(newTemp));
    return false;
  }
  if (newLight < 0 || newLight > 1023) {
    Serial.println("⚠ Light level out of range: " + String(newLight));
    return false;
  }
  
  // Update sensor data
  sensorData.temperature = newTemp;
  sensorData.lightLevel = newLight;
  sensorData.timestamp = newTime;
  sensorData.receivedTime = millis();
  sensorData.packetNumber = newPacket;
  sensorData.arduinoID = newID;
  sensorData.dataValid = true;
  sensorData.dataCount++;
  
  return true;
}

bool parseHeartbeat(String data) {
  Serial.println("💓 HEARTBEAT received from Arduino");
  systemStatus.arduinoConnected = true;
  systemStatus.lastDataReceived = millis();
  return true;
}

bool parseTestData(String data) {
  Serial.println("🧪 TEST data received: " + data);
  return true;
}

bool parseCustomData(String data) {
  Serial.println("🔧 CUSTOM data received: " + data);
  // Bisa parsing custom data format
  return true;
}

void sendHeartbeat() {
  String heartbeat = "STATUS:ONLINE,UPTIME:" + String(millis()) + ",PACKETS:" + String(systemStatus.totalPacketsReceived) + ";";
  arduinoSerial.println(heartbeat);
  Serial.println(">>> HEARTBEAT: " + heartbeat);
}

void checkArduinoConnection() {
  unsigned long timeSinceLastData = millis() - systemStatus.lastDataReceived;
  
  if (timeSinceLastData > DATA_TIMEOUT && systemStatus.arduinoConnected) {
    Serial.println();
    Serial.println("⚠ ARDUINO CONNECTION TIMEOUT!");
    Serial.println("Last data received " + String(timeSinceLastData / 1000) + " seconds ago");
    systemStatus.arduinoConnected = false;
    sensorData.dataValid = false;
  }
}

// ===== DATA DISPLAY =====
void printSensorData() {
  Serial.println("┌─────────────────────────────────────────┐");
  Serial.println("│             SENSOR DATA                 │");
  Serial.println("├─────────────────────────────────────────┤");
  Serial.printf("│ Temperature: %8.2f°C               │\n", sensorData.temperature);
  Serial.printf("│ Light Level: %8d                 │\n", sensorData.lightLevel);
  if (sensorData.packetNumber > 0) {
    Serial.printf("│ Packet #:    %8d                 │\n", sensorData.packetNumber);
  }
  if (sensorData.arduinoID.length() > 0) {
    Serial.printf("│ Arduino ID:  %-15s       │\n", sensorData.arduinoID.c_str());
  }
  Serial.printf("│ Arduino Time:%8lu ms           │\n", sensorData.timestamp);
  Serial.printf("│ Received:    %8lu ms           │\n", sensorData.receivedTime);
  Serial.printf("│ Data Count:  %8d                 │\n", sensorData.dataCount);
  Serial.println("└─────────────────────────────────────────┘");
}

void displayStatistics() {
  unsigned long uptime = millis() - systemStatus.startTime;
  
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║              SYSTEM STATISTICS            ║");
  Serial.println("╠═══════════════════════════════════════════╣");
  Serial.printf("║ Uptime:         %8lu ms           ║\n", uptime);
  Serial.printf("║ Total Packets:  %8d                 ║\n", systemStatus.totalPacketsReceived);
  Serial.printf("║ Valid Data:     %8d                 ║\n", sensorData.dataCount);
  Serial.printf("║ Errors:         %8d                 ║\n", systemStatus.errorCount);
  Serial.printf("║ Arduino Status: %-15s       ║\n", systemStatus.arduinoConnected ? "CONNECTED" : "DISCONNECTED");
  if (systemStatus.lastError.length() > 0) {
    Serial.println("║ Last Error:                               ║");
    Serial.println("║ " + systemStatus.lastError.substring(0, 41) + " ║");
  }
  Serial.println("╚═══════════════════════════════════════════╝");
}

// ===== SERIAL COMMANDS =====
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "stats" || command == "status") {
      displayStatistics();
      
    } else if (command == "reset") {
      Serial.println("🔄 Resetting statistics...");
      systemStatus.totalPacketsReceived = 0;
      systemStatus.errorCount = 0;
      systemStatus.lastError = "";
      sensorData.dataCount = 0;
      systemStatus.startTime = millis();
      Serial.println("✓ Statistics reset complete");
      
    } else if (command == "help") {
      Serial.println();
      Serial.println("╔═══════════════════════════════════════════╗");
      Serial.println("║              AVAILABLE COMMANDS           ║");
      Serial.println("╠═══════════════════════════════════════════╣");
      Serial.println("║ stats  - Show system statistics          ║");
      Serial.println("║ reset  - Reset counters                  ║");
      Serial.println("║ help   - Show this help                  ║");
      Serial.println("║ test   - Send test command to Arduino    ║");
      Serial.println("║ info   - Show current sensor data        ║");
      Serial.println("╚═══════════════════════════════════════════╝");
      
    } else if (command == "test") {
      Serial.println("📤 Sending test command to Arduino...");
      arduinoSerial.println("CMD:TEST;");
      
    } else if (command == "info") {
      if (sensorData.dataValid) {
        printSensorData();
      } else {
        Serial.println("⚠ No valid sensor data available");
      }
      
    } else if (command.length() > 0) {
      Serial.println("❌ Unknown command: " + command);
      Serial.println("💡 Type 'help' for available commands");
    }
  }
}

// ===== UTILITY FUNCTIONS =====
void updateSystemStatus() {
  systemStatus.uptime = millis() - systemStatus.startTime;
}

void updateStatusLED() {
  static unsigned long lastToggle = 0;
  static bool ledState = false;
  
  if (systemStatus.arduinoConnected && sensorData.dataValid) {
    // Slow blink - receiving data
    if (millis() - lastToggle > 2000) {
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, !ledState); // Inverted logic
      lastToggle = millis();
    }
  } else if (systemStatus.arduinoConnected) {
    // Medium blink - connected but no valid data
    if (millis() - lastToggle > 1000) {
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, !ledState); // Inverted logic
      lastToggle = millis();
    }
  } else {
    // Fast blink - not connected
    if (millis() - lastToggle > 300) {
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, !ledState); // Inverted logic
      lastToggle = millis();
    }
  }
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN_PIN, LOW);  // LED ON (inverted)
    delay(delayMs);
    digitalWrite(LED_BUILTIN_PIN, HIGH); // LED OFF (inverted)
    if (i < times - 1) delay(delayMs);
  }
}
