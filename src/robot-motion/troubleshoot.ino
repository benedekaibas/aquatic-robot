#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Motor control setup
const int ST1_S2 = 4;
SoftwareSerial SWSerial(NOT_A_PIN, 10); // TX only
SabertoothSimplified ST(SWSerial);

// Motor speed
const int MOTOR_POWER = 60;  // Gentle descent

void setup() {
  Serial.begin(9600);
  SWSerial.begin(9600);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  // Initialize sensor
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);
  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Starting descent...");
}

void loop() {
  // Run motor gently downward
  digitalWrite(ST1_S2, HIGH);
  ST.motor(1, MOTOR_POWER);
  delayMicroseconds(50);
  digitalWrite(ST1_S2, LOW);

  // Read and report depth
  sensor.read();
  float depth = sensor.depth();
  Serial.print("📏 Depth: ");
  Serial.print(depth);
  Serial.println(" m");

  delay(1000);  // Read every second
}
