/*****************************************************************************
 * | File        : LCDTypes.h
 * | Author      : Refactored to OOP
 * | Function    : Common types, enums, and color definitions for LCD library
 * | Info        : Shared definitions used by WaveshareLCD and LCDTouch classes
 *****************************************************************************/

#ifndef __LCD_TYPES_H
#define __LCD_TYPES_H

#include <stdint.h>

//------------------------------------------------------------------------------
// Basic type definitions
//------------------------------------------------------------------------------
typedef uint16_t COLOR;
typedef uint16_t POINT;
typedef uint16_t LENGTH;

//------------------------------------------------------------------------------
// LCD dimensions
//------------------------------------------------------------------------------
#define LCD_X_MAXPIXEL  480
#define LCD_Y_MAXPIXEL  320
#define LCD_X           0
#define LCD_Y           0
#define LCD_WIDTH       (LCD_X_MAXPIXEL - 2 * LCD_X)
#define LCD_HEIGHT      LCD_Y_MAXPIXEL

//------------------------------------------------------------------------------
// Scan direction enumeration
//------------------------------------------------------------------------------
enum class ScanDir : uint8_t {
    L2R_U2D = 0,    // Left to Right, Up to Down
    L2R_D2U,        // Left to Right, Down to Up
    R2L_U2D,        // Right to Left, Up to Down
    R2L_D2U,        // Right to Left, Down to Up
    U2D_L2R,        // Up to Down, Left to Right
    U2D_R2L,        // Up to Down, Right to Left
    D2U_L2R,        // Down to Up, Left to Right
    D2U_R2L         // Down to Up, Right to Left
};

#define SCAN_DIR_DEFAULT ScanDir::D2U_L2R

//------------------------------------------------------------------------------
// Drawing enumerations
//------------------------------------------------------------------------------
enum class DotPixel : uint8_t {
    PX_1X1 = 1,
    PX_2X2,
    PX_3X3,
    PX_4X4,
    PX_5X5,
    PX_6X6,
    PX_7X7,
    PX_8X8
};

#define DOT_PIXEL_DEFAULT DotPixel::PX_1X1

enum class DotStyle : uint8_t {
    FILL_AROUND = 1,
    FILL_RIGHTUP
};

#define DOT_STYLE_DEFAULT DotStyle::FILL_AROUND

enum class LineStyle : uint8_t {
    SOLID = 0,
    DOTTED
};

enum class DrawFill : uint8_t {
    EMPTY = 0,
    FULL
};

//------------------------------------------------------------------------------
// Color definitions (RGB565 format)
//------------------------------------------------------------------------------
namespace Colors {
    constexpr COLOR WHITE   = 0xFFFF;
    constexpr COLOR BLACK   = 0x0000;
    constexpr COLOR BLUE    = 0x001F;
    constexpr COLOR BRED    = 0xF81F;
    constexpr COLOR GRED    = 0xFFE0;
    constexpr COLOR GBLUE   = 0x07FF;
    constexpr COLOR RED     = 0xF800;
    constexpr COLOR MAGENTA = 0xF81F;
    constexpr COLOR GREEN   = 0x07E0;
    constexpr COLOR CYAN    = 0x7FFF;
    constexpr COLOR YELLOW  = 0xFFE0;
    constexpr COLOR BROWN   = 0xBC40;
    constexpr COLOR BRRED   = 0xFC07;
    constexpr COLOR GRAY    = 0x8430;
}

// Default colors
#define LCD_BACKGROUND      Colors::WHITE
#define FONT_BACKGROUND     Colors::WHITE
#define FONT_FOREGROUND     Colors::GRED

//------------------------------------------------------------------------------
// Time structure for clock display
//------------------------------------------------------------------------------
struct DevTime {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
};

//------------------------------------------------------------------------------
// LCD display info structure
//------------------------------------------------------------------------------
struct LCDInfo {
    LENGTH  width;
    LENGTH  height;
    ScanDir scanDir;
    POINT   xAdjust;
    POINT   yAdjust;
};

//------------------------------------------------------------------------------
// Default pin configuration for ESP32
//------------------------------------------------------------------------------
struct LCDPins {
    uint8_t cs;
    uint8_t rst;
    uint8_t dc;
    uint8_t bl;

    // Default constructor with ESP32 Thing Plus pins
    LCDPins(uint8_t csPin = 15, uint8_t rstPin = 26,
            uint8_t dcPin = 17, uint8_t blPin = 0)
        : cs(csPin), rst(rstPin), dc(dcPin), bl(blPin) {}
};

struct TouchPins {
    uint8_t cs;
    uint8_t irq;
    uint8_t busy;

    // Default constructor with ESP32 Thing Plus pins
    TouchPins(uint8_t csPin = 4, uint8_t irqPin = 16, uint8_t busyPin = 12)
        : cs(csPin), irq(irqPin), busy(busyPin) {}
};

#endif // __LCD_TYPES_H
