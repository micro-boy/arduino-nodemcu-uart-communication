/*
  Arduino Sensor Data Sender with Dummy Data
  
  Mengirim data sensor dummy (suhu dan cahaya) ke NodeMCU
  menggunakan SoftwareSerial untuk komunikasi UART
  
  Hardware:
  - Arduino Uno/Nano/Mega
  - Level Converter (5V ↔ 3.3V)
  - LED indicator (optional)
  
  Connections:
  - Pin 2 Arduino (TX) -> Level Converter HV1 -> NodeMCU D2 (RX)
  - Pin 3 Arduino (RX) -> Level Converter HV2 -> NodeMCU D1 (TX)
  - Pin 0,1 reserved for Serial Monitor debugging
  - Pin 13 -> LED indicator (optional)
  
  Data Generated:
  - Temperature: 20-35°C with realistic variations
  - Light: 100-900 with day/night simulation
  - Packet counter and timestamp included
  
  Author: Your Name
  Date: 2025
  Version: 1.0
*/

#include <SoftwareSerial.h>

// ===== PIN DEFINITIONS =====
#define LED_INDICATOR_PIN 13  // LED indikator komunikasi

// Pin untuk komunikasi serial dengan NodeMCU
#define ARDUINO_TX_PIN 2      // Pin 2 Arduino -> NodeMCU RX
#define ARDUINO_RX_PIN 3      // Pin 3 Arduino <- NodeMCU TX

// ===== SERIAL COMMUNICATION =====
SoftwareSerial nodeSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

// ===== VARIABLES =====
float temperature = 25.0;    // Data dummy suhu
int lightLevel = 500;        // Data dummy cahaya
unsigned long lastSendTime = 0;
unsigned long lastHeartbeat = 0;
bool communicationStatus = false;
int dataPacketCounter = 0;

// ===== TIMING CONSTANTS =====
const unsigned long SEND_INTERVAL = 2000;      // Kirim data setiap 2 detik
const unsigned long HEARTBEAT_INTERVAL = 10000; // Heartbeat setiap 10 detik
const unsigned long RESPONSE_TIMEOUT = 1000;    // Timeout menunggu respon 1 detik

// ===== DUMMY DATA GENERATION =====
float baseTemperature = 25.0;
int baseLightLevel = 500;
float tempTrend = 0.1;
int lightTrend = 5;

// ===== SETUP FUNCTION =====
void setup() {
  // Inisialisasi Serial Monitor untuk debugging
  Serial.begin(9600);
  Serial.println();
  Serial.println("==================================");
  Serial.println("  Arduino Dummy Sensor Sender");
  Serial.println("==================================");
  Serial.println("Menginisialisasi sistem...");
  
  // Inisialisasi komunikasi dengan NodeMCU
  nodeSerial.begin(9600);
  Serial.println("SoftwareSerial initialized pada pin 2,3");
  
  // Setup pin LED
  pinMode(LED_INDICATOR_PIN, OUTPUT);
  
  // Test LED indicator
  Serial.println("Testing LED indicator...");
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_INDICATOR_PIN, HIGH);
    delay(200);
    digitalWrite(LED_INDICATOR_PIN, LOW);
    delay(200);
  }
  
  // Inisialisasi random seed
  randomSeed(analogRead(A0));
  
  Serial.println("Sistem siap! Mulai mengirim data dummy...");
  Serial.println("----------------------------------");
  
  // Test komunikasi awal
  testCommunication();
}

// ===== MAIN LOOP =====
void loop() {
  // Generate dan kirim data sensor dummy secara berkala
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    generateDummyData();
    sendSensorData();
    lastSendTime = millis();
  }
  
  // Kirim heartbeat secara berkala
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
  
  // Cek respon dari NodeMCU
  checkNodeMCUResponse();
  
  // Update status LED
  updateStatusLED();
  
  delay(100); // Delay kecil untuk stabilitas
}

// ===== DUMMY DATA GENERATION =====
void generateDummyData() {
  // Generate dummy temperature (20-35°C dengan variasi realistis)
  float tempVariation = random(-200, 201) / 100.0; // -2.0 to +2.0
  temperature = baseTemperature + tempVariation;
  
  // Trend temperature untuk simulasi perubahan cuaca
  baseTemperature += tempTrend;
  if (baseTemperature > 35.0 || baseTemperature < 20.0) {
    tempTrend = -tempTrend; // Reverse trend
  }
  
  // Generate dummy light level (100-900 dengan variasi)
  int lightVariation = random(-50, 51); // -50 to +50
  lightLevel = baseLightLevel + lightVariation;
  
  // Trend light untuk simulasi siang/malam
  baseLightLevel += lightTrend;
  if (baseLightLevel > 900 || baseLightLevel < 100) {
    lightTrend = -lightTrend; // Reverse trend
  }
  
  // Pastikan nilai dalam range yang valid
  temperature = constrain(temperature, 20.0, 35.0);  // Force ke range yang benar
  lightLevel = constrain(lightLevel, 100, 900);       // Force ke range yang benar
  
  // Debug info
  Serial.print("Generated Dummy - Suhu: ");
  Serial.print(temperature, 2);
  Serial.print("°C, Cahaya: ");
  Serial.print(lightLevel);
  Serial.print(" (Packet #");
  Serial.print(dataPacketCounter + 1);
  Serial.println(")");
}

void sendSensorData() {
  dataPacketCounter++;
  
  // Format data: "TEMP:25.30,LIGHT:512,TIME:123456,PACKET:001;"
  String dataPacket = "TEMP:" + String(temperature, 2) + 
                     ",LIGHT:" + String(lightLevel) + 
                     ",TIME:" + String(millis()) + 
                     ",PACKET:" + String(dataPacketCounter, DEC) +
                     ",ID:ARDUINO01;";
  
  // Kirim data ke NodeMCU
  nodeSerial.println(dataPacket);
  
  // Log ke Serial Monitor
  Serial.println(">>> Mengirim: " + dataPacket);
  
  // Indikator pengiriman
  blinkLED(1, 100);
}

// ===== COMMUNICATION FUNCTIONS =====
void sendHeartbeat() {
  String heartbeat = "HEARTBEAT:" + String(millis()) + ",STATUS:ACTIVE;";
  nodeSerial.println(heartbeat);
  Serial.println(">>> Heartbeat: " + heartbeat);
}

void testCommunication() {
  Serial.println("Testing komunikasi dengan NodeMCU...");
  nodeSerial.println("TEST:ARDUINO_READY,VERSION:1.0;");
  
  // Tunggu respon
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    if (nodeSerial.available()) {
      String response = nodeSerial.readString();
      response.trim();
      Serial.println("<<< Test Response: " + response);
      if (response.indexOf("OK") >= 0) {
        Serial.println("✓ Komunikasi berhasil!");
        communicationStatus = true;
        return;
      }
    }
    delay(100);
  }
  Serial.println("⚠ Tidak ada respon dari NodeMCU");
}

void checkNodeMCUResponse() {
  if (nodeSerial.available()) {
    String response = nodeSerial.readStringUntil('\n');
    response.trim();
    
    if (response.length() > 0) {
      Serial.println("<<< Respon NodeMCU: " + response);
      
      // Proses berbagai jenis respon
      if (response == "OK" || response.indexOf("OK") >= 0) {
        communicationStatus = true;
        Serial.println("✓ Data berhasil diterima NodeMCU");
        blinkLED(2, 50); // 2x blink cepat
        
      } else if (response == "ERROR" || response.indexOf("ERROR") >= 0) {
        communicationStatus = false;
        Serial.println("✗ Error dari NodeMCU: " + response);
        blinkLED(5, 100); // 5x blink error
        
      } else if (response.indexOf("STATUS") >= 0) {
        Serial.println("ℹ Status update dari NodeMCU: " + response);
        
      } else if (response.indexOf("CMD") >= 0) {
        processCommand(response);
      }
    }
  }
}

void processCommand(String command) {
  Serial.println("📤 Memproses perintah: " + command);
  
  if (command.indexOf("RESET") >= 0) {
    Serial.println("🔄 Reset data dummy...");
    baseTemperature = 25.0;
    baseLightLevel = 500;
    dataPacketCounter = 0;
    
  } else if (command.indexOf("SPEED") >= 0) {
    Serial.println("⚡ Speed test mode...");
    // Bisa implementasi mode pengiriman cepat untuk testing
    
  } else if (command.indexOf("TEMP_RANGE") >= 0) {
    // Contoh: CMD:TEMP_RANGE:20-40; untuk set range suhu
    Serial.println("🌡️ Temperature range command received");
    
  } else if (command.indexOf("LIGHT_RANGE") >= 0) {
    // Contoh: CMD:LIGHT_RANGE:100-800; untuk set range cahaya  
    Serial.println("💡 Light range command received");
  }
}

// ===== UTILITY FUNCTIONS =====
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_INDICATOR_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_INDICATOR_PIN, LOW);
    if (i < times - 1) delay(delayMs);
  }
}

void updateStatusLED() {
  static unsigned long lastToggle = 0;
  static bool ledState = false;
  
  if (communicationStatus) {
    // LED slow blink jika komunikasi OK
    if (millis() - lastToggle > 2000) {
      ledState = !ledState;
      digitalWrite(LED_INDICATOR_PIN, ledState);
      lastToggle = millis();
    }
  } else {
    // LED fast blink jika ada masalah komunikasi
    if (millis() - lastToggle > 500) {
      ledState = !ledState;
      digitalWrite(LED_INDICATOR_PIN, ledState);
      lastToggle = millis();
    }
  }
}

// ===== ADDITIONAL DUMMY FUNCTIONS =====
void setDummyTemperatureRange(float minTemp, float maxTemp) {
  Serial.println("Setting temperature range: " + String(minTemp) + " to " + String(maxTemp));
  // Implementasi untuk set range suhu dummy
}

void setDummyLightRange(int minLight, int maxLight) {
  Serial.println("Setting light range: " + String(minLight) + " to " + String(maxLight));
  // Implementasi untuk set range cahaya dummy
}

void sendCustomDummyData(float customTemp, int customLight) {
  String customPacket = "CUSTOM:TEMP:" + String(customTemp, 2) + 
                       ",LIGHT:" + String(customLight) + 
                       ",TIME:" + String(millis()) + ";";
  nodeSerial.println(customPacket);
  Serial.println(">>> Custom Dummy: " + customPacket);
}

void generateExtremeData() {
  // Generate data ekstrem untuk testing
  temperature = random(0, 1) ? -10.0 : 50.0;  // Suhu ekstrem
  lightLevel = random(0, 1) ? 0 : 1023;       // Cahaya ekstrem
  Serial.println("🔥 Generated extreme dummy data for testing");
}
