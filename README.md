# Optical Encoder System

## Overview
This project implements a complete optical encoder system for measuring motor speed, RPM, and distance traveled. The system uses IR-based optical encoders connected to microcontroller interrupt pins to count rotational pulses and calculate real-time speed metrics.

## Project Features
- **Dual Implementation**: 
  - `main.cpp` - Serial communication interface for real-time monitoring
  - `opticalEncoderI2CSlave.cpp` - I2C slave interface for integration with master devices (e.g., Raspberry Pi)
- **Real-time Calculations**: RPM, instantaneous speed, and averaged speed metrics
- **Debouncing**: Hardware debouncing (3ms) to prevent false pulse counting
- **Moving Average Filtering**: 7-sample moving average for smooth RPM readings
- **Speed Calculations**: Conversion from RPM to km/h using configurable wheel diameter
- **Interrupt-driven**: Efficient pulse counting using hardware interrupts with atomic operations
- **Thread-safe**: Safe access to shared variables between interrupt and main loop

## Hardware Requirements
- Microcontroller with I2C support and hardware interrupts (ESP32, Arduino with interrupt pins)
- Optical Encoder (128 pulses per revolution - configurable)
- IR Sensor module connected to interrupt pin (default: GPIO 13/PIN 13)
- I2C communication interface (SDA: GPIO 21, SCL: GPIO 22)

## Configuration
Edit these constants in the code to match your setup:
```cpp
#define PIN_ENC 13                  // Interrupt pin for encoder
#define I2C_SLAVE_ADDR 0x01        // I2C slave address
const int PULSES_PER_REV = 128;    // Encoder resolution
const int UPDATE_INTERVAL_MS = 50; // Update frequency
const int NUM_READINGS = 7;        // Moving average window
```

## Compilation & Deployment
- **Platform**: Arduino IDE or PlatformIO with ESP32 framework
- **Baud Rate**: 115200 (Serial communication)
- **I2C Pins**: SDA=21, SCL=22 (configurable in Wire.begin())

## Data Output
### Serial Interface (main.cpp)
```
RPM: 1234.56 | Avg RPM: 1200.34 | Speed (km/h): 45.67 | Avg Speed (km/h): 44.32
```

### I2C Interface (opticalEncoderI2CSlave.cpp)
Master device requests data from slave address 0x01, receives formatted speed string.

## Calculations
- **RPM**: (Pulses / Resolution) / Time × 60
- **Speed (km/h)**: RPM / 60 × π × WheelDiameter × 3.6
- **Rotations**: Total Pulses / Resolution

## Notes
- The system operates at 60 Hz update rate in I2C mode (16.67ms) and 20 Hz in serial mode (50ms)
- Debounce threshold is set to 3ms to handle switch bounce
- Wheel diameter is estimated at 0.33m; adjust the calculation in the code for different wheel sizes
