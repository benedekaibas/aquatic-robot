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

// State
enum DepthState {
  GOING_DOWN_TO_3M,
  REACHED_3M,
  GOING_UP_TO_2M,
  HOLDING_AT_2M,
  GOING_UP_TO_1M,
  HOLDING_AT_1M,
  GOING_TO_SURFACE,
  STOP_AT_PH_CHANGE,
  DONE
};

DepthState currentState = GOING_DOWN_TO_3M;

// Timing
unsigned long holdStartTime = 0;
const unsigned long holdDuration = 5000; // 10 seconds

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
    delay(300); // Slower sensor reads
  }

  if (count == 0) return false;
  depth = (sum / count) + offset;
  return true;
}

bool getPhChange(float &depth, int samples = 3, float offset = 0.5) {
  float sum = 0.0;
  int count = 0;
  const float phChange = 1.3 // this is where the pH significantly changes in the water
  
  for(int i = 0; i < samples; ++i) {
    sensor.read();
    float sensor_depth = sensor.depth();

    if (sensor_depth > -5.0 && sensor_depth < 20) {
      sum += sensor_depth;
      count++
    }
    delay(300);
  }
  if(count == 0) return false;
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

    case GOING_DOWN_TO_3M:
      if (depth < 3.0) {
        engine(3, 75);  // Slow descent
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 3m. Ascending to 2m...");
        currentState = GOING_UP_TO_2M;
      }
      break;

    case GOING_UP_TO_2M:
      if (depth > 2.2) {
        engine(3, -127); // Fast ascent
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 2m. Holding...");
        holdStartTime = millis();
        currentState = HOLDING_AT_2M;
      }
      break;

    case HOLDING_AT_2M:
      engine(3, 0);
      if (millis() - holdStartTime >= holdDuration) {
        Serial.println("🕒 Hold at 2m complete. Ascending to 1m...");
        currentState = GOING_UP_TO_1M;
      }
      break;

    case GOING_UP_TO_1M:
      if (depth > 1.2) {
        engine(3, -127); // Fast ascent
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 1m. Holding...");
        holdStartTime = millis();
        currentState = HOLDING_AT_1M;
      }
      break;

    case HOLDING_AT_1M:
      engine(3, 0);
      if (millis() - holdStartTime >= holdDuration) {
        Serial.println("🕒 Hold at 1m complete. Surfacing...");
        currentState = GOING_TO_SURFACE;
      }
      break;

    case GOING_TO_SURFACE:
      if (depth > 0.3) {
        engine(3, -127); // Continue ascent
      } else {
        engine(3, 0);
        Serial.println("✅ Surfaced. Mission complete.");
        currentState = DONE;
      }
      break;

    case DONE:
      engine(3, 0);
      delay(1000);
      break;
  }

  delay(200); // Prevent I2C overload
}

