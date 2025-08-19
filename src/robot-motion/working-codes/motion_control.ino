#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>


/*
  Motion control for the robot with a 0.5 meters increments between checking points. This code makes sure that our robot moves up and down between
  different levels in the water in a structured and precise way.
*/

// Sensor and communication
MS5837 sensor;
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10); // TX only
SabertoothSimplified ST(SWSerial);

// Motor control pin
const int ST1_S2 = 4;

// State
enum DepthState {
  GOING_DOWN_TO_4_5M,
  HOLDING_AT_4M,
  HOLDING_AT_3_5M,
  HOLDING_AT_3M,
  HOLDING_AT_2_5M,
  HOLDING_AT_2M,
  HOLDING_AT_1_5M,
  HOLDING_AT_1M,
  HOLDING_AT_0_5M,
  GOING_TO_SURFACE,
  DONE
};

DepthState currentState = GOING_DOWN_TO_4_5M;

// Timing
unsigned long holdStartTime = 0;
const unsigned long holdDuration = 5000; // 5 seconds

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

void controlDepthCycle() {
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

    case GOING_DOWN_TO_4_5M:
      if (depth < 4.5) {
        engine(3, 75);  // Slow descent
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 4.5m. Ascending to 4m...");
        holdStartTime = millis();
        currentState = HOLDING_AT_4M;
      }
      break;

    case HOLDING_AT_4M:
      if (depth > 4.2) {
        engine(3, -100); // Ascent to 4m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 4m complete. Ascending to 3.5m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_3_5M;
        }
      }
      break;

    case HOLDING_AT_3_5M:
      if (depth > 3.7) {
        engine(3, -100); // Ascent to 3.5m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 3.5m complete. Ascending to 3m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_3M;
        }
      }
      break;

    case HOLDING_AT_3M:
      if (depth > 3.2) {
        engine(3, -100); // Ascent to 3m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 3m complete. Ascending to 2.5m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_2_5M;
        }
      }
      break;

    case HOLDING_AT_2_5M:
      if (depth > 2.7) {
        engine(3, -100); // Ascent to 2.5m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 2.5m complete. Ascending to 2m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_2M;
        }
      }
      break;

    case HOLDING_AT_2M:
      if (depth > 2.2) {
        engine(3, -100); // Ascent to 2m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 2m complete. Ascending to 1.5m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_1_5M;
        }
      }
      break;

    case HOLDING_AT_1_5M:
      if (depth > 1.7) {
        engine(3, -100); // Ascent to 1.5m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 1.5m complete. Ascending to 1m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_1M;
        }
      }
      break;

    case HOLDING_AT_1M:
      if (depth > 1.2) {
        engine(3, -100); // Ascent to 1m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 1m complete. Ascending to 0.5m...");
          holdStartTime = millis();
          currentState = HOLDING_AT_0_5M;
        }
      }
      break;

    case HOLDING_AT_0_5M:
      if (depth > 0.7) {
        engine(3, -100); // Ascent to 0.5m
      } else {
        engine(3, 0);
        if (millis() - holdStartTime >= holdDuration) {
          Serial.println("🕒 Hold at 0.5m complete. Surfacing...");
          currentState = GOING_TO_SURFACE;
        }
      }
      break;

    case GOING_TO_SURFACE:
      if (depth > 0.3) {
        engine(3, -127); // Continue ascent to surface
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

  Serial.println("✅ System initialized. Starting descent to 4.5m...");
}

void loop() {
  controlDepthCycle();
}
