# MPU6050 Accelerometer Display

ESP32 application that visualizes MPU6050 accelerometer data on a WaveShare LCD display.

## Features

- **X/Y Acceleration Arrow**: Shows direction and magnitude of acceleration in the X/Y plane
- **Z Acceleration Arrow**: Shows direction and magnitude of vertical (Z-axis) acceleration
- **Temperature Display**: Shows MPU6050 temperature in Celsius (upper-right corner)
- **Real-time Magnitude**: Displays acceleration magnitude in g-forces below each arrow
- **Smooth Animation**: Low-pass filtering reduces jitter for clean visualization

## Hardware Setup

- ESP32 Development Board
- MPU6050 IMU Sensor (I2C: SDA=GPIO23, SCL=GPIO22)
- WaveShare LCD Display

## Understanding the Variables

### Raw Sensor Data
- **ax, ay, az**: Raw accelerometer readings from MPU6050 (16-bit integers)
  - Range: typically ±16384 = ±2g (where 1g = 9.8 m/s² = Earth's gravity)
  - **At rest on a table**: az ≈ 16384 (gravity pulling down), ax ≈ 0, ay ≈ 0

### Filtered Data
- **fax, fay, faz**: Low-pass filtered versions of ax, ay, az
  - Smooths out jitter and noise using exponential moving average
  - ALPHA = 0.85 means 85% old value + 15% new value
  - This is what actually controls the arrows on screen

### Screen Positions
- **cx, cy**: Center coordinates (the origin point where arrows start)
- **px, py**: Pixel position where the X/Y arrow points to
  - `px = cx + (fax * 0.008)` - moves right when tilted right
  - `py = cy - (fay * 0.008)` - moves up when tilted forward (note the minus sign)
- **zy**: Pixel position for the Z arrow
  - `zy = cy - (faz * 0.010)` - moves up when face-up, down when face-down

## MPU6050 Orientation

The MPU6050 coordinate system (component side = top):

```
              ┌─────────────┐
              │   MPU6050   │
              │      ▲      │  ← +Y axis (forward)
              │      │      │
              │   ┌──┼──┐   │
      +X ←────┼───│  ●  │   │  (● = chip center)
  (right)     │   └─────┘   │
              │             │
              └─────────────┘
                     ▲
                  +Z axis (up/out of chip)
```

## How to Test - Physical Movements

### X/Y Arrow (Right Side Display)

**Start Position**: Place device flat on table
- Arrow should be near center (small due to gravity mostly on Z axis)

**Tilt RIGHT** (rotate around Y axis):
- X increases → arrow points RIGHT
- Example: ax goes from 0 to +5000 → arrow moves right

**Tilt LEFT**:
- X decreases → arrow points LEFT
- Example: ax goes to -5000 → arrow moves left

**Tilt FORWARD** (rotate around X axis):
- Y increases → arrow points UP (due to the minus sign in `py = cy - fay`)
- Example: ay goes to +5000 → arrow points up

**Tilt BACKWARD**:
- Y decreases → arrow points DOWN
- Example: ay goes to -5000 → arrow points down

**Diagonal Tilts**:
- Combine RIGHT + FORWARD → arrow points UP-RIGHT
- Combine LEFT + BACKWARD → arrow points DOWN-LEFT

### Z Arrow (Left Side Display)

**Flat on Table** (face up):
- az ≈ +16384 (1g) → arrow points UP strongly

**Flip Upside Down** (face down):
- az ≈ -16384 (-1g) → arrow points DOWN strongly

**Stand on Edge**:
- az ≈ 0 → arrow near center (no Z acceleration)

## Quick Test Sequence

Try these movements to see each arrow clearly:

1. **Flat on table** → Small X/Y arrow (center), Large Z arrow pointing UP
2. **Tilt 45° to the right** → X/Y arrow points RIGHT
3. **Tilt 45° forward** → X/Y arrow points UP
4. **Tilt 45° diagonally** (right+forward) → X/Y arrow points UP-RIGHT
5. **Stand device on edge** → Large X or Y acceleration, Z arrow near center
6. **Flip upside down** → Z arrow flips to point DOWN

## Example Log Reading

```
ax=2568  ay=-900  az=22332
```

This means:
- Device is **tilted slightly right** (ax=2568 is positive)
- Device is **tilted slightly backward** (ay=-900 is negative)
- Device is mostly **face up** (az=22332 is large and positive, close to 1g)

## Key Concept

The arrows show the **direction and magnitude of the acceleration vector** - which is primarily Earth's gravity (1g ≈ 9.8 m/s²) when the device is stationary. When you tilt the device, you're changing which axis feels gravity's pull!

## Building and Uploading

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Code Structure

- `src/main.cpp` - Main application logic
- Low-pass filter reduces sensor noise
- Selective redrawing prevents screen flicker
- Event-driven logging only on movement
- Temperature updates every 1 second

## Troubleshooting

**Arrows not moving?**
- Check I2C connections (SDA, SCL, VCC, GND)
- Verify MPU6050 connection message in serial monitor
- Try adjusting `scaleXY` and `scaleZ` sensitivity values

**Flickering display?**
- Increase `MOVE_THRESH` to reduce update frequency
- Adjust `ALPHA` filter value (higher = more smoothing)

**Temperature seems off?**
- MPU6050 temperature sensor is approximate
- Formula: `tempC = (rawTemp / 340.0) + 36.53`
- Mainly useful for relative changes, not absolute accuracy
