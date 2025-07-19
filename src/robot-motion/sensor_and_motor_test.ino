#include <Wire.h>
#include "MS5837.h"

// === Motor Includes ===
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

// === Sensor ===
MS5837 sensor;

// === Motor Setup ===
const int ST1_S2 = 4; // Motor selection pin
SoftwareSerial SWSerial(NOT_A_PIN, 10); // TX only
SabertoothSimplified ST(SWSerial);

// === Timing ===
unsigned long lastReadTime = 0;
const unsigned long interval = 5000; // 5 seconds

// === Motor Control ===
void engine(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  SWSerial.begin(9600);          // Motor serial
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  // Initialize sensor
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // Freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Depth test initialized. Reading every 5 seconds...");

  // Start motor for interference testing
  delay(1000);  // Optional startup delay
  Serial.println("⚙️ Motor starting for interference test...");
  engine(3, 100);  // Constant motor thrust (e.g., going down)
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastReadTime >= interval) {
    lastReadTime = currentTime;

    sensor.read();
    float depth = sensor.depth();
    float temperature = sensor.temperature();

    Serial.print("🌊 Depth (m): ");
    Serial.print(depth);
    Serial.print(" | 🌡 Temp (°C): ");
    Serial.println(temperature);
  }
}
