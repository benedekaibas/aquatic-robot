#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

// Sensor
MS5837 sensor;

// Motor setup
const int ST1_S2 = 4; // Motor select pin
SoftwareSerial SWSerial(NOT_A_PIN, 10); // TX only
SabertoothSimplified ST(SWSerial);

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
  SWSerial.begin(9600);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  // Sensor config
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // Freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Dive mode initialized. Starting descent...");
  engine(3, 50);  // Start motor at low speed downward
}

void loop() {
  sensor.read();

  float depth = sensor.depth();
  float temperature = sensor.temperature();

  Serial.print("🌊 Depth (m): ");
  Serial.print(depth);
  Serial.print(" | 🌡 Temp (°C): ");
  Serial.println(temperature);

  delay(1000);  // Read every second
}
