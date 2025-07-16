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

// Motor Control
void engine(int motorNum, int power = 127) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

// Safe sensor read with basic filtering
bool safeSensorRead(float &depth, float offset, int retries = 3) {
  for (int i = 0; i < retries; ++i) {
    sensor.read();
    float raw = sensor.depth();
    float final = raw + offset;

    Serial.print("Raw: ");
    Serial.print(raw);
    Serial.print(" + Offset: ");
    Serial.print(offset);
    Serial.print(" = Final depth: ");
    Serial.println(final);

    if (final > -5.0 && final < 20.0) {
      depth = final;
      return true;
    }

    delay(20);  // delay between retries
  }

  Serial.println("❌ Failed to get valid depth after retries");
  depth = -999.0;
  return false;
}

// Main control logic for depth state transitions
void controlDepthCycle() {
  float depth;
  float depthOffset = 0.5;
  const float tolerance = 0.05;

  if (!safeSensorRead(depth, depthOffset)) {
    engine(3, 0);
    Serial.println("Invalid depth reading. Halting.");
    return;
  }

  switch (currentState) {
    case GOING_DOWN:
      if (depth < 3.0 - tolerance) {
        engine(3, 100);  // Go down
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
        engine(3, -100);  // Go up
      } else {
        engine(3, 0);
        Serial.println("Reached 1m. Going back to 3m.");
        currentState = GOING_DOWN;
      }
      break;
  }
}

void setup() {
  Serial.begin(SerialBaudRate);
  SWSerial.begin(SerialBaudRate);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  // Initialize sensor model and fluid density BEFORE init
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // Fresh water

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
  delay(20);  // Prevents hammering the sensor
}
