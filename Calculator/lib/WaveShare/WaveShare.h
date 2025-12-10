#ifndef WAVEESHARE_H
#define WAVEESHARE_H

#include <Arduino.h>
#include "WaveshareLCD.h"
#include "LCDTouch.h"

/**
 * WaveShare - Simplified LCD interface wrapper
 *
 * Wraps the WaveshareLCD and LCDTouch classes into a single
 * easy-to-use interface for the calculator application.
 */
class WaveShare {
public:
    WaveShare();
    ~WaveShare() = default;

    // Initialization
    void begin();

    // Screen properties
    uint16_t getWidth() const;
    uint16_t getHeight() const;

    // Screen operations
    void fillScreen(uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // Text operations
    void drawText(int16_t x, int16_t y, const char* text, sFONT* font,
                  uint16_t bgColor, uint16_t fgColor);
    void drawTextCentered(int16_t x, int16_t y, int16_t w, int16_t h,
                          const char* text, sFONT* font,
                          uint16_t bgColor, uint16_t fgColor);
    void drawTextRightAligned(int16_t x, int16_t y, int16_t w,
                              const char* text, sFONT* font,
                              uint16_t bgColor, uint16_t fgColor);

    // Touch operations
    bool readTouch(int16_t& tx, int16_t& ty);
    bool isTouched() const;

    // Direct access to underlying objects (for advanced use)
    WaveshareLCD& getLCD() { return _lcd; }
    LCDTouch& getTouch() { return _touch; }

private:
    WaveshareLCD _lcd;
    LCDTouch _touch;

    // Touch calibration values (for ESP32 Thing Plus + Waveshare 3.5")
    static constexpr float TOUCH_X_FAC = -0.132443f;
    static constexpr float TOUCH_Y_FAC = 0.089997f;
    static constexpr int TOUCH_X_OFF = 516;
    static constexpr int TOUCH_Y_OFF = -22;
};

#endif // WAVEESHARE_H
