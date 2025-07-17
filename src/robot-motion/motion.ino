#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Pin Definitions
const int ST1_S2 = 4;

// Communication
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10);
SabertoothSimplified ST(SWSerial);

// State
enum DepthState { GOING_DOWN, HOLDING_AT_BOTTOM, GOING_UP };
DepthState currentState = GOING_DOWN;

unsigned long holdStartTime = 0;
bool holdStarted = false;

void engine(int motorNum, int power = 127) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

void measureDepth(float &depth, float depthOffset) {
  sensor.read();
  float raw = sens.depth(); // store raw reading for debugging
  depth = raw + depthOffset;

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" + Offset: ");
  Serial.print(depthOffset);
  Serial.print(" = Final depth: ");
  Serial.print(depth);

  if (depth < -5.0 || depth > 20.0) {
    Serial.println("Sensor read failed or returned bad value");
    depth = -999.0;
  } else {
    Serial.print("Depth value is valid");
  }
}

void controlDepthCycle() {
  float depth;
  float depthOffset = 0.5;
  const float tolerance = 0.05;

  measureDepth(depth, depthOffset);

  if (depth == -999.0) {
    engine(3, 0); //Stops motor
    Serial.println("Invalid depth reading. Halting.");
    return;
  }

  switch (currentState) {
    case GOING_DOWN:
      if (depth < 3.0 - tolerance) {
        engine(3, 100);  // Move down
      } else {
        engine(3, 0);    // Stop
        Serial.println("Reached 3m. Holding...");
        holdStartTime = millis();
        holdStarted = true;
        currentState = HOLDING_AT_BOTTOM;
      }
      break;

    case HOLDING_AT_BOTTOM:
      engine(3, 0);  // Stay still
      if (holdStarted && millis() - holdStartTime >= 10000) {
        Serial.println("Hold complete. Going back to 1m.");
        holdStarted = false;
        currentState = GOING_UP;
      }
      break;

    case GOING_UP:
      if (depth > 1.0 + tolerance) {
        engine(3, -100) // Move up (negative power)
      } else {
        engine(3, 0);
        Serial.print("Reached 1m. Going back to 3m.");
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

  if (!sensor.init()) {
    Serial.println("MS5837 not found!");
    while (true){
      delay(1000); //this part is updated for removing error message MS5837 not found
    }
  }

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);
  // sensor.setOversampling(MS5837::OSR_8192);
  // sensor.begin();

  Serial.println("System initialized. Starting depth loop...");
}

void loop() {
  controlDepthCycle();
}



