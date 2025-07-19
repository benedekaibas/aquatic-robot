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

// Timing
unsigned long lastReadTime = 0;
const unsigned long interval = 5000; // 5 seconds

// Motor helper function
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

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // Freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Depth test initialized. Reading every 5 seconds...");

  // 🟢 Turn on motor with minimal power
  Serial.println("⚙️ Motor starting with minimal power...");
  engine(3, 10);  // Low forward power
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
