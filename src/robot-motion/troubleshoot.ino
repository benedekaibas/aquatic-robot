#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

// Sensor and communication
MS5837 sensor;
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10); // TX only
SabertoothSimplified ST(SWSerial);

// Motor control pin
const int ST1_S2 = 4;

// State machine
enum DepthState { GOING_DOWN, REACHED_TARGET, GOING_UP, DONE };
DepthState currentState = GOING_DOWN;

void setup() {
  Serial.begin(SerialBaudRate);
  SWSerial.begin(SerialBaudRate);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // Freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ System initialized. Starting...");
}

void engine(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

bool getFilteredDepth(float &depth, int samples = 3, float offset = 0.5) {
  float sum = 0;
  int count = 0;

  for (int i = 0; i < samples; i++) {
    sensor.read();
    float d = sensor.depth();
    if (d > -5.0 && d < 20.0) {
      sum += d;
      count++;
    }
    delay(200); // Increased delay to reduce sensor workload
  }

  if (count == 0) return false;
  depth = (sum / count) + offset;
  return true;
}

void loop() {
  float depth;
  if (!getFilteredDepth(depth)) {
    Serial.println("❌ Invalid depth reading");
    engine(3, 0);
    delay(500);
    return;
  }

  Serial.print("📏 Depth: ");
  Serial.println(depth, 2);

  switch (currentState) {
    case GOING_DOWN:
      if (depth < 1.0) {
        engine(3, 75); // Dive slowly
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 1m. Surfacing...");
        currentState = GOING_UP;
      }
      break;

    case GOING_UP:
      if (depth > 0.3) {
        engine(3, -127); // Ascend with full power
      } else {
        engine(3, 0);
        Serial.println("✅ Surfaced.");
        currentState = DONE;
      }
      break;

    case DONE:
      engine(3, 0);
      delay(1000); // Idle
      break;
  }

  delay(200); // Further I2C throttling
}
