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
- WaveShareLCD - LCD display with touch (OOP library)

## Code style

- C++ with Arduino framework
- Source files in `src/`, headers in `include/`
- OOP design pattern for hardware libraries (see WaveShareLCD)

---

## WaveShareLCD Library (OOP)

Object-oriented library for Waveshare 3.5" TFT LCD with touch (ILI9486 + XPT2046).

### Library Files

```text
lib/WaveshareLCD/src/
├── LCDTypes.h        # Common types, enums, colors, pin configs
├── WaveshareLCD.h    # LCD driver + GUI class
├── WaveshareLCD.cpp
├── LCDTouch.h        # Touch panel class
├── LCDTouch.cpp
└── fonts/            # Font data (8, 12, 16, 20, 24 pt)
```

### Quick Start

```cpp
#include "WaveshareLCD.h"
#include "LCDTouch.h"

WaveshareLCD lcd;           // Default pins
LCDTouch touch(lcd);

void setup() {
    lcd.begin();            // Initialize LCD
    touch.begin();          // Initialize touch
    lcd.clear(Colors::WHITE);
}

void loop() {
    if (touch.scan() && touch.isPressed()) {
        lcd.drawPoint(touch.getX(), touch.getY(), Colors::BLUE);
    }
}
```

### Custom Pin Configuration

```cpp
LCDPins lcdPins(15, 26, 17, 0);      // CS, RST, DC, BL
TouchPins touchPins(4, 16, 12);      // CS, IRQ, BUSY

WaveshareLCD lcd(lcdPins);
LCDTouch touch(lcd, touchPins);
```

### WaveshareLCD Class Methods

| Method | Description |
|--------|-------------|
| `begin(scanDir, backlight)` | Initialize LCD |
| `clear(color)` | Clear screen |
| `getWidth()` / `getHeight()` | Get dimensions |
| `setBacklight(value)` | Set backlight (0-255) |
| `drawPoint(x, y, color, size, style)` | Draw point |
| `drawLine(x1, y1, x2, y2, color, style, size)` | Draw line |
| `drawRectangle(x1, y1, x2, y2, color, fill, size, style)` | Draw rectangle |
| `drawCircle(x, y, radius, color, fill, size)` | Draw circle |
| `drawString(x, y, str, font, bg, fg)` | Draw text |
| `drawNumber(x, y, num, font, bg, fg)` | Draw number |
| `fillArea(x1, y1, x2, y2, color)` | Fill rectangular area |

### LCDTouch Class Methods

| Method | Description |
|--------|-------------|
| `begin()` | Initialize touch panel |
| `scan(calibrationMode)` | Scan for touch, returns true if touched |
| `isPressed()` | Check if currently pressed |
| `getX()` / `getY()` | Get touch coordinates |
| `setColor(color)` | Set drawing color (for paint apps) |
| `calibrate()` | Run calibration |

### Type-Safe Enums

```cpp
ScanDir::L2R_U2D    // Left-to-Right, Up-to-Down (and 7 others)
DotPixel::PX_1X1    // through PX_8X8
LineStyle::SOLID    // or DOTTED
DrawFill::EMPTY     // or FULL
DotStyle::FILL_AROUND  // or FILL_RIGHTUP
```

### Colors (namespace Colors::)

`WHITE`, `BLACK`, `RED`, `GREEN`, `BLUE`, `YELLOW`, `CYAN`, `MAGENTA`, `GRAY`, `BROWN`, `BRED`, `GRED`, `GBLUE`, `BRRED`

### Default Pins (ESP32 Thing Plus)

| Function | LCD Pin | Touch Pin |
|----------|---------|-----------|
| CS       | 15      | 4         |
| RST      | 26      | -         |
| DC       | 17      | -         |
| BL       | 0       | -         |
| IRQ      | -       | 16        |
| BUSY     | -       | 12        |
