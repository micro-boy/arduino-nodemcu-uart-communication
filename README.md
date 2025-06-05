# Arduino NodeMCU UART Communication

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-00979D?style=flat&logo=Arduino&logoColor=white)](https://www.arduino.cc/)
[![ESP8266](https://img.shields.io/badge/ESP8266-E7352C?style=flat&logo=espressif&logoColor=white)](https://www.espressif.com/)

Komunikasi UART dua arah antara Arduino dan NodeMCU (ESP8266) menggunakan Logic Level Converter dengan data dummy sensor untuk testing dan development.

## 📋 Deskripsi

Project ini mendemonstrasikan komunikasi serial yang stabil antara Arduino (5V) dan NodeMCU (3.3V) melalui bidirectional logic level converter. Arduino mengirim data dummy sensor (suhu dan cahaya) ke NodeMCU yang kemudian memproses dan menampilkan data tersebut di Serial Monitor.

### ✨ Fitur Utama

- 🔄 **Komunikasi UART Bidirectional** dengan SoftwareSerial
- 📊 **Data Dummy Realistis** (suhu 20-35°C, cahaya 100-900)
- 🔧 **Level Converter Support** untuk komunikasi 5V ↔ 3.3V
- 📱 **Real-time Monitoring** via Serial Monitor
- 📈 **Statistik Komunikasi** (packets, errors, uptime)
- 💡 **LED Status Indicators** untuk debugging visual
- 🛠️ **Error Handling** dan data validation
- ⌨️ **Interactive Commands** untuk control dan testing

## 🔧 Hardware Requirements

### Komponen Utama
- **Arduino Uno/Nano/Mega** (5V)
- **NodeMCU ESP8266** (3.3V) 
- **Bidirectional Logic Level Converter** (3.3V ↔ 5V)
- **Breadboard** dan kabel jumper
- **Power supply** (USB atau external)

### Komponen Opsional
- **LED** + resistor 220Ω (indikator Arduino)
- **Multimeter** (untuk troubleshooting)

## 📐 Wiring Diagram

```
Arduino (5V)    Level Converter    NodeMCU (3.3V)
─────────────────────────────────────────────────
GND        ──── GND          ──── GND
5V         ──── HV           ──── 3.3V
Pin 2 (TX) ──── HV1     LV1  ──── D2 (GPIO4) RX
Pin 3 (RX) ──── HV2     LV2  ──── D1 (GPIO5) TX
Pin 13     ──── [LED + R]    ──── Built-in LED
```

### Pin Configuration
- **Arduino**: SoftwareSerial pada pin 2,3 | Serial Monitor pada pin 0,1
- **NodeMCU**: SoftwareSerial pada D1,D2 | Serial Monitor via USB

## 🚀 Installation

### Prerequisites
- Arduino IDE dengan ESP8266 board package
- Driver USB untuk Arduino dan NodeMCU

### Steps
1. **Clone repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/arduino-nodemcu-uart-communication.git
   cd arduino-nodemcu-uart-communication
   ```

2. **Setup Arduino IDE**
   - Install ESP8266 board package via Board Manager
   - Select correct board: Arduino Uno dan NodeMCU 1.0

3. **Upload Code**
   - Upload `arduino_sender.ino` ke Arduino board
   - Upload `nodemcu_receiver.ino` ke NodeMCU board

4. **Hardware Setup**
   - Wiring sesuai diagram di atas
   - Pastikan semua koneksi GND terhubung
   - Double-check polaritas level converter

## 📱 Usage

### Starting the System
1. **Power up** kedua board
2. **Open Serial Monitor** untuk Arduino (9600 baud)
3. **Open Serial Monitor** untuk NodeMCU (9600 baud)
4. **Monitor komunikasi** real-time

### Arduino Output Example
```
==================================
  Arduino Dummy Sensor Sender
==================================
Generated Dummy - Suhu: 24.73°C, Cahaya: 456 (Packet #1)
>>> Mengirim: TEMP:24.73,LIGHT:456,TIME:5234,PACKET:1,ID:ARDUINO01;
<<< Respon NodeMCU: OK
✓ Data berhasil diterima NodeMCU
```

### NodeMCU Output Example
```
<<< RECEIVED: TEMP:24.73,LIGHT:456,TIME:5234,PACKET:1,ID:ARDUINO01
>>> SENT: OK
┌─────────────────────────────────────────┐
│             SENSOR DATA                 │
├─────────────────────────────────────────┤
│ Temperature:    24.73°C                 │
│ Light Level:      456                   │
│ Packet #:           1                   │
│ Arduino ID:  ARDUINO01                  │
└─────────────────────────────────────────┘
```

### NodeMCU Interactive Commands
Ketik di Serial Monitor NodeMCU:
- `stats` - Tampilkan statistik sistem
- `reset` - Reset counter dan statistik  
- `help` - Daftar command tersedia
- `test` - Kirim test command ke Arduino

## 📁 Code Structure

```
├── arduino_sender/
│   └── arduino_sender.ino          # Arduino main code
├── nodemcu_receiver/
│   └── nodemcu_receiver.ino        # NodeMCU main code
├── docs/
│   ├── wiring_diagram.png          # Visual wiring guide
│   └── level_converter_guide.md    # Level converter details
├── examples/
│   ├── basic_test.ino              # Simple communication test
│   └── advanced_features.ino       # Extended functionality
└── README.md
```

## 🔍 Protocol Details

### Data Format
**Arduino → NodeMCU:**
```
TEMP:25.30,LIGHT:512,TIME:123456,PACKET:001,ID:ARDUINO01;
```

**NodeMCU → Arduino:**
```
OK                    # Success
ERROR:PARSE_FAILED    # Parse error
STATUS:ONLINE         # Status update
```

### Message Types
- `TEMP:` - Data sensor dummy
- `HEARTBEAT:` - Keep-alive signal
- `TEST:` - Testing communication
- `CMD:` - Commands and controls

## 🔧 Configuration

### Arduino Settings
```cpp
// Timing configuration
const unsigned long SEND_INTERVAL = 2000;      // Data interval (ms)
const unsigned long HEARTBEAT_INTERVAL = 10000; // Heartbeat interval (ms)

// Dummy data ranges
float baseTemperature = 25.0;  // Base temperature (°C)
int baseLightLevel = 500;      // Base light level (0-1023)
```

### NodeMCU Settings
```cpp
// Communication timeouts
const unsigned long DATA_TIMEOUT = 10000;    // Connection timeout (ms)
const unsigned long STATS_INTERVAL = 15000;  // Stats display interval (ms)
```

## 🐛 Troubleshooting

### Common Issues

#### No Communication
- ✅ Check level converter wiring (HV1↔LV1, HV2↔LV2)
- ✅ Verify GND connections
- ✅ Confirm baud rate match (9600)
- ✅ Check TX/RX pin assignments

#### Data Parse Errors
- ✅ Monitor Serial output for malformed packets
- ✅ Check data terminator (`;`) presence
- ✅ Verify data ranges (temp: -50 to 150°C, light: 0-1023)

#### Level Converter Issues
- ✅ Measure voltages: HV=5V, LV=3.3V
- ✅ Check power supply connections
- ✅ Test with multimeter continuity

### LED Status Indicators

**Arduino (Pin 13):**
- 🐌 Slow blink (2s): Communication OK
- ⚡ Fast blink (0.5s): Communication problems
- ✨ Quick flash: Data transmission

**NodeMCU (Built-in):**
- 🐌 Slow blink (2s): Receiving valid data
- 🔄 Medium blink (1s): Connected, invalid data
- ⚡ Fast blink (0.3s): No Arduino connection

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Guidelines
1. **Fork** the repository
2. **Create** feature branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** changes (`git commit -m 'Add AmazingFeature'`)
4. **Push** to branch (`git push origin feature/AmazingFeature`)
5. **Open** Pull Request

### Code Style
- Use descriptive variable names
- Comment complex logic sections
- Follow Arduino IDE formatting conventions
- Test on actual hardware before submitting

## 📋 Roadmap

- [ ] **JSON Protocol** support untuk data complex
- [ ] **Multiple sensor types** simulation
- [ ] **Wireless communication** option (WiFi, RF)
- [ ] **Data logging** ke SD card atau SPIFFS
- [ ] **Web interface** untuk monitoring (opsional)
- [ ] **Mobile app** integration
- [ ] **Error injection** untuk robustness testing

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors

- **Anton Prafanto** - *microboy* - [YourGitHub](https://github.com/micro-boy)

## 🙏 Acknowledgments

- Arduino community untuk dokumentasi dan examples
- ESP8266 community untuk library support
- Logic level converter tutorials dan guides
- Serial communication best practices references

## 📞 Support

Jika mengalami masalah atau ada pertanyaan:

1. **Check** troubleshooting section di atas
2. **Search** existing issues di GitHub
3. **Create** new issue dengan detail lengkap
4. **Include** Serial Monitor output dan wiring photos

---

⭐ **Star this repo** jika project ini membantu Anda!

📖 **Read the docs** untuk informasi lebih detail di folder `/docs`

🔧 **Try the examples** di folder `/examples` untuk quick start
