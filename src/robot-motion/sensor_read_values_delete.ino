#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Pins
const int ST1_S2 = 4;

// Communication
const int SerialBaudRate = 9600;
SoftwareSerial SWSerial(NOT_A_PIN, 10);  // TX only
SabertoothSimplified ST(SWSerial);

// Motor control
void engine(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

void setup() {
  Serial.begin(SerialBaudRate);
  SWSerial.begin(SerialBaudRate);
  Wire.begin();
  pinMode(ST1_S2, OUTPUT);

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Sensor initialized. Turning on motor...");
  engine(3, 100);  // Turn on motor at constant forward power
}

void loop() {
  sensor.read();
  float depth = sensor.depth();

  Serial.print("Depth: ");
  Serial.println(depth);

  delay(5000);  // Wait 5 seconds between readings
}
