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

int debugCounter = 0;

// Motor control helper
void engine(int motorNum, int power = 127) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

// Moving average depth reader (uses raw MS5837 values)
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
    delay(10);  // pause between reads
  }

  if (valid > 0) {
    depth = (sum / valid) + offset;

    // Limit debug output to every 10th read
    if (debugCounter++ % 10 == 0) {
      Serial.print("Avg depth from ");
      Serial.print(valid);
      Serial.print(" samples: ");
      Serial.println(depth);
    }
    return true;
  } else {
    Serial.println("❌ Failed to get valid depth samples");
    depth = -999.0;
    return false;
  }
}

// Controls depth logic based on state machine
void controlDepthCycle() {
  float depth;
  const float tolerance = 0.05;
  static float lastValidDepth = 0.0;
  static int depthFailCount = 0;
  const int maxConsecutiveFails = 30;

  if (!getAverageDepth(depth)) {

    depthFailCount++;
    Serial.print("⚠️ Depth read failed. Using last good depth: ");
    Serial.println(lastValidDepth);

  if (depthFailCount >= maxConsecutiveFails) {
    engine(3, 0);
    Serial.println("❌ Too many bad readings. Halting.");
    return;
  }

  depth = lastValidDepth;  // Use fallback
  } else {
    lastValidDepth = depth;
    depthFailCount = 0;  // Reset fail counter
}

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
  delay(20);  // Prevents I2C spamming
}