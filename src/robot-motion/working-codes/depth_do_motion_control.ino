#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>
#include <math.h>  // for fabsf

// ====== DO change marker config ======
const float DO_MARKER = 1.5f;          // depth where DO changes significantly
const float DO_BAND   = 0.05f;         // tolerance band
const unsigned long DO_COOLDOWN_MS = 2000;

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
  DONE
};

DepthState currentState = GOING_DOWN_TO_3M;

// Timing
unsigned long holdStartTime = 0;
const unsigned long holdDuration = 5000; // 5 seconds

// Internal tracking
float prevDepth = NAN;
unsigned long lastDoNotifyMs = 0;

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

bool getFilteredDepth(float &depth, int samples = 3, float offset = 0.5f) {
  float sum = 0;
  int count = 0;

  for (int i = 0; i < samples; i++) {
    sensor.read();
    float d = sensor.depth();
    if (d > -5.0f && d < 20.0f) {
      sum += d;
      count++;
    }
    delay(300); // Slower sensor reads
  }

  if (count == 0) return false;
  depth = (sum / count) + offset;
  return true;
}

static inline bool crossedMarker(float from, float to, float marker, float band) {
  // Detects a crossing of 'marker' between depths 'from' -> 'to' with a small band to avoid noise
  // Cross if the segment straddles the marker (opposite sides) and moved more than band.
  if (isnan(from)) return false;
  if (fabsf(to - from) < band) return false;
  const bool before = (from < marker - band);
  const bool after  = (to   < marker - band);
  const bool beforeHigh = (from > marker + band);
  const bool afterHigh  = (to   > marker + band);

  // Cross upward: from deeper than (marker+band) to shallower than (marker-band)
  if (beforeHigh && !afterHigh && to <= marker + band) return true;
  // Cross downward: from shallower than (marker-band) to deeper than (marker+band)
  if (!before && after && to >= marker - band) return true;

  // Generic sign change check (fallback)
  if ((from - marker) * (to - marker) <= 0) return true;

  return false;
}

void maybeLogDoMarker(float depth) {
  if (crossedMarker(prevDepth, depth, DO_MARKER, DO_BAND)) {
    unsigned long now = millis();
    if (now - lastDoNotifyMs >= DO_COOLDOWN_MS) {
      const char* dir = (depth < prevDepth) ? "⬆️ ascending" : "⬇️ descending";
      Serial.print("🔶 DO marker crossed near ");
      Serial.print(DO_MARKER, 2);
      Serial.print(" m (");
      Serial.print(dir);
      Serial.print("). Current depth: ");
      Serial.print(depth, 2);
      Serial.println(" m");
      lastDoNotifyMs = now;
    }
  }
}

void loop() {
  float depth;
  if (!getFilteredDepth(depth)) {
    Serial.println("❌ Invalid depth reading");
    engine(3, 0);
    delay(500);
    return;
  }

  // Log DO marker crossings (no behavior change)
  maybeLogDoMarker(depth);

  Serial.print("📏 Depth: ");
  Serial.println(depth, 2);

  switch (currentState) {
    case GOING_DOWN_TO_3M:
      if (depth < 3.0f) {
        engine(3, 75);  // Slow descent
      } else {
        engine(3, 0);
        Serial.println("✅ Reached 3m. Ascending to 2m...");
        currentState = GOING_UP_TO_2M;
      }
      break;

    case GOING_UP_TO_2M:
      if (depth > 2.2f) {
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
      if (depth > 1.2f) {
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
      if (depth > 0.3f) {
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

  prevDepth = depth; // update after logic
  delay(200); // Prevent I2C overload
}
