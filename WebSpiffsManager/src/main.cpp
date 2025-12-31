// // ============================================================================
// // MPU6050 Accelerometer Visualization for ESP32 + WaveShare LCD
// // ============================================================================
// // This program reads acceleration data from an MPU6050 sensor and displays:
// //   1. X/Y Arrow: Shows direction and magnitude of tilt in the X/Y plane
// //   2. Z Arrow: Shows direction and magnitude of vertical (up/down) acceleration
// //   3. Temperature: Displays MPU6050 internal temperature in Celsius
// //
// // How it works:
// //   - Raw sensor data (ax, ay, az) is read from MPU6050 via I2C
// //   - Low-pass filter smooths the data to reduce jitter (fax, fay, faz)
// //   - Filtered values are scaled and converted to pixel positions
// //   - Arrows are drawn from center point (cx, cy) to calculated endpoints (px, py, zy)
// //   - Only redraws when movement exceeds threshold (prevents flicker)
// //
// // Coordinate system (MPU6050 chip facing up):
// //   +X = Right, +Y = Forward, +Z = Up
// //   At rest flat on table: ax≈0, ay≈0, az≈16384 (gravity = 1g)
// // ============================================================================

// #include <Arduino.h>
// #include "WaveShareDemo.h"   // LCD_Driver / LCD_GUI / LCD_Touch
// #include "I2Cdev.h"
// #include "MPU6050.h"

// // Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// // is used in I2Cdev.h
// #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//     #include "Wire.h"
// #endif

// // ============================================================================
// // MPU6050 SENSOR VARIABLES
// // ============================================================================
// // class default I2C address is 0x68
// // specific I2C addresses may be passed as a parameter here
// // AD0 low = 0x68 (default for InvenSense evaluation board)
// // AD0 high = 0x69
// MPU6050 accelgyro;
// //MPU6050 accelgyro(0x69); // <-- use for AD0 high
// //MPU6050 accelgyro(0x68, &Wire1); // <-- use for AD0 low, but 2nd Wire (TWI/I2C) object

// // Raw sensor readings (16-bit integers from MPU6050)
// // Range: ±16384 represents ±2g (where 1g = 9.8 m/s² = Earth's gravity)
// int16_t ax, ay, az;   // Accelerometer X, Y, Z axes
// int16_t gx, gy, gz;   // Gyroscope X, Y, Z axes (not currently used for display)

// // uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// // list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// // not so easy to parse, and slow(er) over UART.
// #define OUTPUT_READABLE_ACCELGYRO

// // uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// // binary, one right after the other. This is very fast (as fast as possible
// // without compression or data loss), and easy to parse, but impossible to read
// // for a human.
// //#define OUTPUT_BINARY_ACCELGYRO


// #define LED_PIN 13
// bool blinkState = false;

// // ============================================================================
// // DISPLAY VARIABLES - Understanding the Calculations
// // ============================================================================

// // Low-pass filter variables (smooths sensor noise)
// // Formula: new_filtered = (ALPHA * old_filtered) + ((1-ALPHA) * new_raw)
// static float fax = 0, fay = 0, faz = 0;           // Filtered acceleration values (smoothed)
// static const float ALPHA = 0.85f;                  // Filter coefficient: 0.85 = keep 85% old, blend 15% new

// // Previous arrow positions (for selective redrawing - prevents flicker)
// static int prevPx = -9999, prevPy = -9999, prevZy = -9999;
// static const int MOVE_THRESH = 3;                  // Minimum pixel movement to trigger redraw

// // Screen layout coordinates
// // cx, cy = center point where arrows originate (the "zero" position)
// // px, py = calculated endpoint for X/Y arrow (changes with tilt)
// // zy = calculated endpoint for Z arrow (changes with up/down orientation)
// static int W, H;                                   // Screen width and height
// static int cx, cy;                                 // Center coordinates for arrow origins
// static int zx = 60;                                // X position for Z axis display (left side)
// static int xyBoxLeft, xyBoxRight, xyBoxTop, xyBoxBottom;  // Boundaries for display areas

// // Temperature update timing
// static unsigned long lastTempUpdate = 0;
// static const unsigned long TEMP_UPDATE_INTERVAL = 1000;  // Update temperature every 1 second

// // ---------------- Drawing helpers ----------------
// static int clampi(int v, int lo, int hi) {
//   if (v < lo) return lo;
//   if (v > hi) return hi;
//   return v;
// }

// // Erase arrow by drawing it in background color
// void EraseArrowXY(int cx, int cy, int px, int py, COLOR color = WHITE) {
//   if (px == -9999) return; // Never drawn before

//   int dx = px - cx;
//   int dy = py - cy;
//   float len = sqrtf((float)dx*dx + (float)dy*dy);

//   // Erase main vector
//   GUI_DrawLine(cx, cy, px, py, color, LINE_SOLID, DOT_PIXEL_3X3);

//   if (len < 1) return;

//   float ang = atan2f((float)dy, (float)dx);
//   const float headAng = 0.5f;
//   const int headLen = 12;

//   int hx1 = px - (int)(cosf(ang - headAng) * headLen);
//   int hy1 = py - (int)(sinf(ang - headAng) * headLen);
//   int hx2 = px - (int)(cosf(ang + headAng) * headLen);
//   int hy2 = py - (int)(sinf(ang + headAng) * headLen);

//   GUI_DrawLine(px, py, hx1, hy1, color, LINE_SOLID, DOT_PIXEL_2X2);
//   GUI_DrawLine(px, py, hx2, hy2, color, LINE_SOLID, DOT_PIXEL_2X2);
// }

// void DrawArrowXY(int cx, int cy, int px, int py) {
//   int dx = px - cx;
//   int dy = py - cy;
//   float len = sqrtf((float)dx*dx + (float)dy*dy);

//   // Main vector
//   GUI_DrawLine(cx, cy, px, py, RED, LINE_SOLID, DOT_PIXEL_3X3);

//   if (len < 1) return;

//   float ang = atan2f((float)dy, (float)dx);
//   const float headAng = 0.5f;
//   const int headLen = 12;

//   int hx1 = px - (int)(cosf(ang - headAng) * headLen);
//   int hy1 = py - (int)(sinf(ang - headAng) * headLen);
//   int hx2 = px - (int)(cosf(ang + headAng) * headLen);
//   int hy2 = py - (int)(sinf(ang + headAng) * headLen);

//   GUI_DrawLine(px, py, hx1, hy1, RED, LINE_SOLID, DOT_PIXEL_2X2);
//   GUI_DrawLine(px, py, hx2, hy2, RED, LINE_SOLID, DOT_PIXEL_2X2);
// }

// void EraseArrowZ(int zx, int cy, int zy, COLOR color = WHITE) {
//   if (zy == -9999) return;

//   GUI_DrawLine(zx, cy, zx, zy, color, LINE_SOLID, DOT_PIXEL_3X3);

//   int headLen = 10;
//   int headWid = 6;

//   if (zy < cy) { // arrow was pointing up
//     // Erase arrowhead lines that were going back DOWN from tip
//     GUI_DrawLine(zx, zy, zx - headWid, zy + headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
//     GUI_DrawLine(zx, zy, zx + headWid, zy + headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
//   } else {       // arrow was pointing down
//     // Erase arrowhead lines that were going back UP from tip
//     GUI_DrawLine(zx, zy, zx - headWid, zy - headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
//     GUI_DrawLine(zx, zy, zx + headWid, zy - headLen, color, LINE_SOLID, DOT_PIXEL_2X2);
//   }
// }

// void DrawArrowZ(int zx, int cy, int zy) {
//   GUI_DrawLine(zx, cy, zx, zy, RED, LINE_SOLID, DOT_PIXEL_3X3);

//   int headLen = 10;
//   int headWid = 6;

//   if (zy < cy) { // arrow pointing up
//     // Draw arrowhead lines going back DOWN from tip (converging /\ shape)
//     GUI_DrawLine(zx, zy, zx - headWid, zy + headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
//     GUI_DrawLine(zx, zy, zx + headWid, zy + headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
//   } else {       // arrow pointing down
//     // Draw arrowhead lines going back UP from tip (converging /\ shape)
//     GUI_DrawLine(zx, zy, zx - headWid, zy - headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
//     GUI_DrawLine(zx, zy, zx + headWid, zy - headLen, RED, LINE_SOLID, DOT_PIXEL_2X2);
//   }
// }

// // Draw static UI elements (axes, labels, borders)
// void DrawStaticUI() {
//   // Draw vertical separator between Z axis and XY plane
//   GUI_DrawLine(zx + 40, 0, zx + 40, H, GRAY, LINE_SOLID, DOT_PIXEL_1X1);

//   // XY plane axes
//   GUI_DrawLine(cx, xyBoxTop, cx, xyBoxBottom, BLACK, LINE_SOLID, DOT_PIXEL_1X1);  // Y axis
//   GUI_DrawLine(xyBoxLeft, cy, xyBoxRight, cy, BLACK, LINE_SOLID, DOT_PIXEL_1X1);   // X axis

//   // Z axis vertical line
//   GUI_DrawLine(zx, xyBoxTop, zx, xyBoxBottom, BLACK, LINE_SOLID, DOT_PIXEL_1X1);

//   // Labels
//   GUI_DisString_EN(cx - 30, xyBoxTop - 20, "X/Y Accel", &Font12, WHITE, BLACK);
//   GUI_DisString_EN(zx - 25, xyBoxTop - 20, "Z Accel", &Font12, WHITE, BLACK);

//   // Axis labels
//   GUI_DisString_EN(xyBoxRight - 10, cy - 20, "X", &Font12, WHITE, GRAY);
//   GUI_DisString_EN(cx + 5, xyBoxTop + 5, "Y", &Font12, WHITE, GRAY);
//   GUI_DisString_EN(zx + 8, xyBoxTop + 5, "Z", &Font12, WHITE, GRAY);

//   // Center dots (prominent markers showing origin points)
//   GUI_DrawCircle(cx, cy, 4, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
//   GUI_DrawCircle(cx, cy, 2, WHITE, DRAW_FULL, DOT_PIXEL_1X1);
//   GUI_DrawCircle(zx, cy, 4, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
//   GUI_DrawCircle(zx, cy, 2, WHITE, DRAW_FULL, DOT_PIXEL_1X1);
// }

// // ---------------- Setup / Loop ----------------
// void setup() {
//   // ---- I2C / MPU6050 ----
//   // join I2C bus (I2Cdev library doesn't do this automatically)
//   #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//       Wire.begin();
//   #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
//       Fastwire::setup(400, true);
//   #endif

//   // initialize serial communication
//   // it's really up to you depending on your project)
//   Serial.begin(115200);
//   Serial.println("\n=== MPU6050 Accelerometer Display ===");

//   // initialize device
//   Serial.println("Initializing I2C/MPU6050 devices...");
//   accelgyro.initialize();

//   // verify connection
//   Serial.println("Testing device connections...");
//   Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

//   // use the code below to change accel/gyro offset values
//   /*
//   Serial.println("Updating internal sensor offsets...");
//   // -76	-2359	1688	0	0	0
//   Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
//   Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
//   Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
//   Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
//   Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
//   Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
//   Serial.print("\n");
//   accelgyro.setXGyroOffset(220);
//   accelgyro.setYGyroOffset(76);
//   accelgyro.setZGyroOffset(-85);
//   Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
//   Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
//   Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
//   Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
//   Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
//   Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
//   Serial.print("\n");
//   */

//   // configure Arduino LED pin for output
//   pinMode(LED_PIN, OUTPUT);

//   // ---- LCD ----
//   Wvshr_Init();
//   LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
//   LCD_Init(Lcd_ScanDir, 200);
//   LCD_Clear(WHITE);
//   TP_Init();

//   // Calculate screen layout
//   W = sLCD_DIS.LCD_Dis_Column;
//   H = sLCD_DIS.LCD_Dis_Page;

//   // XY display area (right side)
//   xyBoxLeft = zx + 45;
//   xyBoxRight = W - 10;
//   xyBoxTop = 35;
//   xyBoxBottom = H - 10;

//   cx = (xyBoxLeft + xyBoxRight) / 2;
//   cy = (xyBoxTop + xyBoxBottom) / 2;

//   // Draw static UI elements once
//   DrawStaticUI();

//   // Initialize filtered values from first read
//   accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
//   fax = ax;
//   fay = ay;
//   faz = az;

//   Serial.println("Setup complete!\n");
// }

// void loop() {
//     // read raw accel/gyro measurements from device
//     accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

//     // these methods (and a few others) are also available
//     //accelgyro.getAcceleration(&ax, &ay, &az);
//     //accelgyro.getRotation(&gx, &gy, &gz);

//     #ifdef OUTPUT_BINARY_ACCELGYRO
//         Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
//         Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
//         Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
//         Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
//         Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
//         Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
//     #endif

//     // blink LED to indicate activity
//     blinkState = !blinkState;
//     digitalWrite(LED_PIN, blinkState);

//     // Apply low-pass filter to smooth jitter
//     // Formula: new_filtered = (ALPHA * old_filtered) + ((1-ALPHA) * new_raw)
//     // ALPHA=0.85 means: keep 85% of old value, blend in 15% of new value
//     // This creates smooth transitions and reduces sensor noise
//     fax = ALPHA * fax + (1.0f - ALPHA) * ax;  // Filtered X acceleration
//     fay = ALPHA * fay + (1.0f - ALPHA) * ay;  // Filtered Y acceleration
//     faz = ALPHA * faz + (1.0f - ALPHA) * az;  // Filtered Z acceleration

//     // Map acceleration to pixel positions
//     // scaleXY: converts raw acceleration units to pixels (adjust for arrow sensitivity)
//     // MPU6050 range: ±16384 = ±2g, so scaleXY=0.008 means ~130 pixels at max tilt
//     float scaleXY = 0.008f;  // Scale for X/Y display
//     float scaleZ = 0.010f;   // Scale for Z display (slightly more sensitive)

//     // Calculate arrow endpoint positions
//     // px: X position = center + (filtered_X_accel * scale)
//     //     - Tilt RIGHT (ax positive) → arrow moves RIGHT
//     //     - Tilt LEFT (ax negative) → arrow moves LEFT
//     int px = cx + (int)(fax * scaleXY);

//     // py: Y position = center - (filtered_Y_accel * scale)
//     //     - MINUS sign inverts Y because screen Y grows downward
//     //     - Tilt FORWARD (ay positive) → arrow points UP (smaller py)
//     //     - Tilt BACKWARD (ay negative) → arrow points DOWN (larger py)
//     int py = cy - (int)(fay * scaleXY);

//     // Clamp to display bounds (prevent drawing outside the box)
//     px = clampi(px, xyBoxLeft + 5, xyBoxRight - 5);
//     py = clampi(py, xyBoxTop + 5, xyBoxBottom - 5);

//     // Calculate Z axis arrow position
//     // zy: Z position = center - (filtered_Z_accel * scale)
//     //     - Device FLAT/FACE-UP (az ≈ +16384) → arrow points UP (smaller zy)
//     //     - Device UPSIDE-DOWN (az ≈ -16384) → arrow points DOWN (larger zy)
//     //     - Device ON EDGE (az ≈ 0) → arrow near center
//     int zy = cy - (int)(faz * scaleZ);
//     zy = clampi(zy, xyBoxTop + 5, xyBoxBottom - 5);

//     // Check if arrows moved significantly
//     bool xyMoved = abs(px - prevPx) > MOVE_THRESH || abs(py - prevPy) > MOVE_THRESH;
//     bool zMoved = abs(zy - prevZy) > MOVE_THRESH;

//     // Update arrows if they moved
//     if (xyMoved) {
//       // Step 1: Erase old arrow with white
//       EraseArrowXY(cx, cy, prevPx, prevPy);

//       // Step 2: Restore the axis lines (but not the center yet)
//       GUI_DrawLine(cx - 15, cy, cx + 15, cy, BLACK, LINE_SOLID, DOT_PIXEL_2X2);
//       GUI_DrawLine(cx, cy - 15, cx, cy + 15, BLACK, LINE_SOLID, DOT_PIXEL_2X2);

//       // Step 3: Draw new arrow
//       DrawArrowXY(cx, cy, px, py);

//       // Step 4: Draw center point ON TOP of the arrow so it's always visible
//       // Use a filled circle that will show on top of the red arrow
//       GUI_DrawCircle(cx, cy, 4, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
//       // Add a white center dot for better visibility
//       GUI_DrawCircle(cx, cy, 2, WHITE, DRAW_FULL, DOT_PIXEL_1X1);

//       prevPx = px;
//       prevPy = py;

//       // Calculate and display magnitude
//       // magXY = vector magnitude in X/Y plane: sqrt(X² + Y²)
//       // Divide by ~16384 to convert to g's, but using 1000 for display scaling
//       // Result shows combined acceleration magnitude (e.g., 1.0g when tilted 45°)
//       float magXY = sqrtf(fax * fax + fay * fay) / 1000.0f;  // in g's (approx)
//       char buf[16];
//       snprintf(buf, sizeof(buf), "%.2fg", magXY);

//       // Clear previous magnitude text area
//       GUI_DrawRectangle(xyBoxRight - 50, xyBoxBottom + 5, xyBoxRight, xyBoxBottom + 20,
//                         WHITE, DRAW_FULL, DOT_PIXEL_1X1);
//       GUI_DisString_EN(xyBoxRight - 48, xyBoxBottom + 7, buf, &Font12, WHITE, BLUE);

//       #ifdef OUTPUT_READABLE_ACCELGYRO
//         Serial.print("[XY moved] px=");
//         Serial.print(px); Serial.print(" py=");
//         Serial.print(py); Serial.print(" magXY=");
//         Serial.println(magXY);
//       #endif
//     }

//     if (zMoved) {
//       // Step 1: Erase old arrow with white
//       EraseArrowZ(zx, cy, prevZy);

//       // Step 2: Restore the axis line (but not the center yet)
//       GUI_DrawLine(zx, cy - 15, zx, cy + 15, BLACK, LINE_SOLID, DOT_PIXEL_2X2);

//       // Step 3: Draw new arrow
//       DrawArrowZ(zx, cy, zy);

//       // Step 4: Draw center point ON TOP of the arrow so it's always visible
//       GUI_DrawCircle(zx, cy, 4, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
//       GUI_DrawCircle(zx, cy, 2, WHITE, DRAW_FULL, DOT_PIXEL_1X1);

//       prevZy = zy;

//       // Calculate and display Z magnitude
//       // magZ = Z acceleration component alone (not vector magnitude)
//       // Positive = facing up, Negative = facing down
//       // At rest flat on table: magZ ≈ +1.0g (gravity pulling down on sensor)
//       float magZ = faz / 1000.0f;  // in g's (approx)
//       char buf[16];
//       snprintf(buf, sizeof(buf), "%.2fg", magZ);

//       // Clear previous magnitude text area
//       GUI_DrawRectangle(zx - 30, xyBoxBottom + 5, zx + 30, xyBoxBottom + 20,
//                         WHITE, DRAW_FULL, DOT_PIXEL_1X1);
//       GUI_DisString_EN(zx - 25, xyBoxBottom + 7, buf, &Font12, WHITE, BLUE);

//       #ifdef OUTPUT_READABLE_ACCELGYRO
//         Serial.print("[Z moved] zy=");
//         Serial.print(zy); Serial.print(" magZ=");
//         Serial.println(magZ);
//       #endif
//     }

//     // Update temperature periodically (not on every frame)
//     unsigned long now = millis();
//     if (now - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
//       lastTempUpdate = now;

//       int16_t tRaw = accelgyro.getTemperature();
//       float tempC = (tRaw / 340.0f) + 36.53f;  // MPU6050 datasheet formula

//       char buf[20];
//       snprintf(buf, sizeof(buf), "%.1f C", tempC);

//       // Clear previous temperature area
//       GUI_DrawRectangle(W - 80, 5, W - 5, 25, WHITE, DRAW_FULL, DOT_PIXEL_1X1);
//       GUI_DisString_EN(W - 75, 10, buf, &Font16, WHITE, BLACK);

//       #ifdef OUTPUT_READABLE_ACCELGYRO
//           // display tab-separated accel/gyro x/y/z values
//           Serial.print("a/g:\t ax=");
//           Serial.print(ax); Serial.print("\t ay=");
//           Serial.print(ay); Serial.print("\t az=");
//           Serial.print(az); Serial.print("\t gx=");
//           Serial.print(gx); Serial.print("\t gy=");
//           Serial.print(prevPy); Serial.print("\t prevPy=");
//           Serial.println(gz);

//           // Debug output to serial
//           Serial.print("Accel: X(fax)=");
//           Serial.print((int)fax);
//           Serial.print(" Y(fay)=");
//           Serial.print((int)fay);
//           Serial.print(" Z(faz)=");
//           Serial.print((int)faz);
//           Serial.print(" | Temp=");
//           Serial.print(tempC);
//           Serial.println("C");
//       #endif
//     }
//     delay(100);
// }
