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

// Motor control
void engine(int motorNum, int power = 127) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

// Get averaged depth with validation
bool getAverageDepth(float &depth, float offset = 0.5, int samples = 5) {
  float sum = 0;
  int valid = 0;

  for (int i = 0; i < samples; ++i) {
    sensor.read();
    float raw = sensor.depth();
    if (raw > -5.0 && raw < 20.0) {
      sum += raw;
      valid++;
    }
    delay(10);
  }

  if (valid > 0) {
    depth = (sum / valid) + offset;
    Serial.print("Avg depth from ");
    Serial.print(valid);
    Serial.print(" samples: ");
    Serial.println(depth);
    return true;
  } else {
    return false;
  }
}

// Control state transitions based on depth
void controlDepthCycle() {
  float depth;
  const float tolerance = 0.05;

  // Keep track of last good reading
  static float lastValidDepth = 0.0;
  static int depthFailCount = 0;
  const int maxDepthFails = 10;

  // Try to get valid depth
  if (!getAverageDepth(depth)) {
    depthFailCount++;
    Serial.print("⚠️ Depth read failed. Using last good depth: ");
    Serial.println(lastValidDepth);

    if (depthFailCount >= maxDepthFails) {
      engine(3, 0);
      Serial.println("❌ Too many bad readings. Halting.");
      return;
    }

    depth = lastValidDepth;  // Use fallback
  } else {
    lastValidDepth = depth;
    depthFailCount = 0;
  }

  // State machine logic
  static bool hasGoneDeepEnough = false;
  static bool hasRisenHighEnough = false;

  switch (currentState) {
    case GOING_DOWN:
      if (depth < 2.95) {
        hasGoneDeepEnough = true;
        engine(3, 100);  // Dive
      } else if (hasGoneDeepEnough && depth >= 2.95) {
        engine(3, 0);
        Serial.println("Reached 3m. Holding...");
        holdStartTime = millis();
        holdStarted = true;
        currentState = HOLDING_AT_BOTTOM;
        hasGoneDeepEnough = false;
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
      if (depth > 1.05) {
        hasRisenHighEnough = true;
        engine(3, -100);  // Surface
      } else if (hasRisenHighEnough && depth <= 1.05) {
        engine(3, 0);
        Serial.println("Reached 1m. Going back to 3m.");
        currentState = GOING_DOWN;
        hasRisenHighEnough = false;
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
  sensor.setFluidDensity(997);  // Freshwater

  if (!sensor.init()) {
    Serial.println("MS5837 not found!");
    while (true) delay(1000);
  }

  Serial.println("System initialized. Starting depth loop...");
}

void loop() {
  controlDepthCycle();
  delay(20);  // Prevent I2C overload
}
