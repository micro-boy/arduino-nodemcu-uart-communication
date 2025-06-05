// Arduino Code - Pengirim Data Sensor
#include <SoftwareSerial.h>

// Pin untuk sensor (contoh: sensor suhu LM35, sensor cahaya LDR, dll)
#define TEMP_SENSOR_PIN A0
#define LIGHT_SENSOR_PIN A1
#define LED_PIN 13

// Pin untuk komunikasi serial dengan NodeMCU
#define ARDUINO_TX_PIN 2  // Pin 2 Arduino -> NodeMCU RX
#define ARDUINO_RX_PIN 3  // Pin 3 Arduino <- NodeMCU TX

// Inisialisasi SoftwareSerial
SoftwareSerial nodeSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

// Variabel untuk menyimpan data sensor
float temperature;
int lightLevel;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000; // Kirim data setiap 2 detik

void setup() {
  // Inisialisasi serial komunikasi
  Serial.begin(9600);      // Untuk Serial Monitor (debugging)
  nodeSerial.begin(9600);  // Untuk komunikasi dengan NodeMCU
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  
  Serial.println("Arduino siap mengirim data...");
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Baca data sensor setiap interval tertentu
  if (millis() - lastSendTime >= sendInterval) {
    // Baca sensor suhu (LM35)
    int tempReading = analogRead(TEMP_SENSOR_PIN);
    temperature = (tempReading * 5.0 * 100.0) / 1024.0; // Konversi ke Celsius
    
    // Baca sensor cahaya (LDR)
    lightLevel = analogRead(LIGHT_SENSOR_PIN);
    
    // Format data untuk dikirim: "TEMP:xx.xx,LIGHT:xxxx;"
    String dataToSend = "TEMP:" + String(temperature, 2) + 
                       ",LIGHT:" + String(lightLevel) + 
                       ",TIME:" + String(millis()) + ";";
    
    // Kirim data ke NodeMCU
    nodeSerial.println(dataToSend);
    
    // Tampilkan di serial monitor untuk debugging
    Serial.println("Data terkirim: " + dataToSend);
    
    // Indikator LED
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    
    lastSendTime = millis();
  }
  
  // Cek apakah ada respon dari NodeMCU
  if (nodeSerial.available()) {
    String response = nodeSerial.readString();
    response.trim();
    Serial.println("Respon dari NodeMCU: " + response);
    
    // Proses respon jika diperlukan
    if (response == "OK") {
      Serial.println("Data berhasil diterima NodeMCU");
    }
  }
  
  delay(100); // Delay kecil untuk stabilitas
}

// Fungsi untuk mengirim data sensor tertentu
void sendSensorData(String sensorType, float value) {
  String data = sensorType + ":" + String(value, 2) + ";";
  nodeSerial.println(data);
  Serial.println("Mengirim: " + data);
}

// Fungsi untuk mengirim perintah ke NodeMCU
void sendCommand(String command) {
  nodeSerial.println("CMD:" + command + ";");
  Serial.println("Mengirim perintah: " + command);
}
