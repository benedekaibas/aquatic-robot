#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Pin Definitions
const int ST1_S2 = 4;

// Communication
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10);  // TX only
SabertoothSimplified ST(SWSerial);

// State
enum DepthState { GOING_DOWN, HOLDING_AT_BOTTOM, GOING_UP };
DepthState currentState = GOING_DOWN;

unsigned long holdStartTime = 0;
bool holdStarted = false;

// Motor control helper
void engine(int motorNum, int power = 127) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

// Direct depth reading (no averaging)
float readDepth(float offset = 0.5) {
  sensor.read();
  float depth = sensor.depth() + offset;

  Serial.print("Depth: ");
  Serial.print(depth);
  Serial.println(" m");

  return depth;
}

/*

// Controls depth logic based on state machine
void controlDepthCycle() {
  float depth = readDepth();  // Direct read
  const float tolerance = 0.05;

  switch (currentState) {
    case GOING_DOWN:
      if (depth < 3.0 - tolerance) {
        engine(3, 100);  // Dive
      } else {
        engine(3, 0);
        Serial.println("Reached 3m. Holding...");
        holdStartTime = millis();
        holdStarted = true;
        currentState = HOLDING_AT_BOTTOM;
      }
      break;

    case HOLDING_AT_BOTTOM:
      engine(3, 0);
      if (holdStarted && millis() - holdStartTime >= 10000) {
        Serial.println("Hold complete. Going back to 1m.");
        holdStarted = false;
        currentState = GOING_UP;
      }
      break;

    case GOING_UP:
      if (depth > 1.0 + tolerance) {
        engine(3, -100);  // Surface
      } else {
        engine(3, 0);
        Serial.println("Reached 1m. Going back to 3m.");
        currentState = GOING_DOWN;
      }
      break;
  }
}
*/

void setup() {
  Serial.begin(SerialBaudRate);
  SWSerial.begin(SerialBaudRate);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // Freshwater

  if (!sensor.init()) {
    Serial.println("MS5837 not found!");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("System initialized. Starting depth loop...");
}

void loop() {
  controlDepthCycle();
  delay(200);  // Slight delay to avoid I2C spamming
}
