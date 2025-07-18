#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Pins
const int ST1_S2 = 4;

// Communication
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10);  // TX only
SabertoothSimplified ST(SWSerial);

// Depth thresholds
const float DEPTH_TOP = 1.0;
const float DEPTH_BOTTOM = 3.0;
const float DEPTH_TOLERANCE = 0.05;
const float DEPTH_OFFSET = 0.5;

// Timers
unsigned long holdStartTime = 0;

// State machine
enum State { GOING_DOWN, HOLDING_BOTTOM, GOING_UP, HOLDING_TOP };
State currentState = GOING_DOWN;

// Last good reading fallback
float lastGoodDepth = 0.0;

// Motor driver helper
void engine(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

// Read safe depth
float readDepth() {
  float sum = 0;
  int valid = 0;

  for (int i = 0; i < 5; i++) {
    if (sensor.read()) {
      float raw = sensor.depth();
      if (raw > -5.0 && raw < 20.0) {
        sum += raw;
        valid++;
      }
    }
    delay(10);
  }

  if (valid > 0) {
    float avgDepth = (sum / valid) + DEPTH_OFFSET;
    lastGoodDepth = avgDepth;
    return avgDepth;
  } else {
    Serial.println("⚠️ Sensor read failed. Using last known depth.");
    return lastGoodDepth;
  }
}

void controlLogic() {
  float depth = readDepth();
  Serial.print("Depth: ");
  Serial.println(depth);

  switch (currentState) {
    case GOING_DOWN:
      if (depth < DEPTH_BOTTOM - DEPTH_TOLERANCE) {
        engine(3, 100);  // Move down
      } else {
        engine(3, 0);
        holdStartTime = millis();
        currentState = HOLDING_BOTTOM;
        Serial.println("✅ Reached 3m. Holding...");
      }
      break;

    case HOLDING_BOTTOM:
      engine(3, 0);
      if (millis() - holdStartTime >= 10000) {
        currentState = GOING_UP;
        Serial.println("🕒 Hold complete. Going up to 1m...");
      }
      break;

    case GOING_UP:
      if (depth > DEPTH_TOP + DEPTH_TOLERANCE) {
        engine(3, -100);  // Move up
      } else {
        engine(3, 0);
        holdStartTime = millis();
        currentState = HOLDING_TOP;
        Serial.println("✅ Reached 1m. Holding...");
      }
      break;

    case HOLDING_TOP:
      engine(3, 0);
      if (millis() - holdStartTime >= 10000) {
        currentState = GOING_DOWN;
        Serial.println("🕒 Hold complete. Going back to 3m...");
      }
      break;
  }
}

void setup() {
  Serial.begin(SerialBaudRate);
  SWSerial.begin(SerialBaudRate);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);

  if (!sensor.init()) {
    Serial.println("❌ MS5837 not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ System initialized. Starting depth loop...");
}

void loop() {
  controlLogic();
  delay(20);  // Smooth loop
}
