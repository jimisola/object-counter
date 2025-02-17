# Arduino Object Counter using TCRT5000 Sensor and TFT Display

## Overview
This Arduino-based project uses a **TCRT5000 infrared sensor** to count detected objects and displays relevant statistics on a **1.8" TFT screen** using the **Adafruit ST7735** library. An optional push button can be used to manually reset counters.

## Features
- **Object Counting:** Increments the counter when the sensor detects an object.
- **TFT Display:** Shows total count, objects per minute/hour, and elapsed time.
- **Auto Reset:** If the sensor is continuously triggered for a set time, the system resets.
- **Manual Reset:** A push button allows manual resetting.

## Components Used
- **Arduino Nano**
- **TCRT5000 Infrared Sensor**
- **1.8" TFT Display (ST7735)**
- **Push Button** for manual reset (optional)

## Wiring
| Component | Arduino Pin |
|-----------|------------|
| TCRT5000 Sensor | `D2` |
| Push Button | `D3` |
| TFT Backlight | `D4` |
| TFT Reset | `D7` |
| TFT DC | `D8` |
| TFT CS | `D9` |
| TFT MOSI | `D11` |
| TFT SCLK | `D13` |

## Configuration
Modify the following constants in the code to adjust behavior:
```cpp
#define TFT_FRAME_RATE 500
#define TFT_BRIGHTNESS 128
#define TCRT500_RESET_THRESHOLD 2000
#define RESET_DELAY 2000
#define DEBOUNCE_TIME_SENSOR 400
#define DEBOUNCE_PUSH_BUTTON 500
```

## Extra

3D model coming on MakerWorld.


