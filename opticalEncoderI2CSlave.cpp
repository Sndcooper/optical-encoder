/*
================================================================================
PROJECT: Optical Encoder I2C Slave Module
FILE: opticalEncoderI2CSlave.cpp
DESCRIPTION: This firmware implements an I2C slave interface for an optical
encoder system. The microcontroller reads pulses from an optical encoder via
interrupt pin and calculates speed/RPM values. It communicates with an I2C
master (e.g., Raspberry Pi) via Wire protocol, sending calculated speed data
upon request. The system features debouncing, moving average filtering for
smooth speed readings, and efficient interrupt-driven pulse counting with
atomic operations to prevent race conditions between interrupt and main loop.
================================================================================
*/

#include <Arduino.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x01
// --- Hardware Mapping ---
#define PIN_ENC 13 // Interrupt input pin
// Variable to send
int finalSpeed = 0;
// --- Calculation Constants ---
const int PULSES_PER_REV = 128; // Change this based on your specific encoder
const int UPDATE_INTERVAL_MS = 1000/60; 

// --- Global Variables ---
volatile uint32_t pulseCount = 0;
uint32_t totalPulses = 0;
float totalRotations = 0.0; 
unsigned long lastUpdateTime = 0;

// --- Debounce Variables ---
volatile unsigned long lastRead = 0;
volatile unsigned long readIR = 0;

// Moving Average Settings
const int NUM_READINGS = 7;      // Average over last 15 readings
float readings[NUM_READINGS];      // The readings from the analog input
int readIndex = 0;                 // The index of the current reading
float totalRPM = 0;                // The running total
float averageRPM = 0;              // The average


// --- Interrupt Service Routine (ISR) ---
void IRAM_ATTR handleEncoder() {
  readIR = millis();
  if((readIR - lastRead) > 3) {
    pulseCount++;
    lastRead = readIR;
  }
}

void onRequest(){
  // When Pi requests data, send the counter value
  // We send it as a string for easy decoding in Python, 
  // or you can send raw bytes.
  Wire.print("speed:"); 
  Wire.print(finalSpeed);
  Wire.print("\n"); // End of line character
}

void setup() {
    Serial.begin(115200);
    
    // Setup Interrupt
    pinMode(PIN_ENC, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC), handleEncoder, RISING);

    Serial.println("--- Serial Monitor Encoder Started ---");
    // Initialize I2C as Slave with address 0x01
    // Default pins: SDA=21, SCL=22
    Wire.begin(I2C_SLAVE_ADDR); 
    
    // Register the function to call when Master asks for data
    Wire.onRequest(onRequest);
    
    Serial.println("I2C Slave Ready");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Update Logic
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL_MS) {
        float deltaTime = (currentTime - lastUpdateTime) / 1000.0; // /1000.0 to convert to seconds
        
        // Atomic snapshot (Pause interrupts briefly to read safely)
        noInterrupts();
        uint32_t currentPulses = pulseCount;
        pulseCount = 0;
        interrupts();

        // Accumulate Totals
        totalPulses += currentPulses;
        unsigned long rotations = currentPulses / PULSES_PER_REV;
        totalRotations = totalPulses / (float)PULSES_PER_REV;
        
        // Math: (Pulses / Resolution) / TimeInSeconds
        float rps = (currentPulses / (float)PULSES_PER_REV) / deltaTime;
        float rpm = rps * 60.000;

        // Subtract the last reading:
        totalRPM = totalRPM - readings[readIndex];
        // Add the new reading:
        readings[readIndex] = rpm;
        // Add the new reading to the total:
        totalRPM = totalRPM + readings[readIndex];
        // Advance to the next position in the array:
        readIndex = readIndex + 1;
        // If we're at the end of the array, wrap around to the beginning:
        if (readIndex >= NUM_READINGS) {
            readIndex = 0;
        }
        // Calculate the average:
        averageRPM = totalRPM / NUM_READINGS;
        float speedMPS = (rps * 0.33 * PI); // Convert RPS to Meters per Second (assuming wheel diameter of 0.33m)
        float avgmps = (averageRPM / 60.0) * 0.33 * PI;
        float speedKPH = speedMPS * 3.6;
        float avgKPH = avgmps * 3.6;

        // Print to Serial Monitor
        Serial.print("RPM: ");
        Serial.print(rpm, 2);
        Serial.print(" | Avg RPM: ");
        Serial.print(averageRPM, 4);
        Serial.print(" | Speed (km/h): ");
        Serial.print(speedKPH, 4);
        Serial.print(" | Avg Speed (km/h): ");
        Serial.print(avgKPH, 4);

        Serial.println();
        speedKPH = abs(speedKPH);
        finalSpeed = static_cast<int>(speedKPH); // Update the variable to send via I2C
        lastUpdateTime = currentTime;
    }
}
