# Embedded Projects

Multi-project PlatformIO workspace for ESP32 development.

## Structure
Each subfolder is an independent PlatformIO project with its own platformio.ini.

## Build
- Use PlatformIO extension in VS Code
- Each project builds separately via PlatformIO: Build/Upload

## Projects
- Blink - Basic LED example
- FirstWebServer - WiFi web server
- I2C_RADAR - I2C radar sensor
- MFRC1 - RFID reader
- MPU6050 - Accelerometer/gyroscope
- WaveShareLCD - LCD display

## Code style
- C++ with Arduino framework
- Source files in `src/`, headers in `include/`
