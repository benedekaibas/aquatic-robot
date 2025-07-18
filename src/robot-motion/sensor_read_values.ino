#include <Wire.h>
#include "MS5837.h"

MS5837 sensor;

// Timing
unsigned long lastReadTime = 0;
const unsigned long interval = 5000; // 5 seconds

// Motor control
void engine(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power);                       // DELETE THIS FUNCTION IF IT'S NOT WORKING
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
} 

void setup() {
  Serial.begin(9600);
  Wire.begin();

  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);  // Freshwater

  if (!sensor.init()) {
    Serial.println("❌ MS5837 sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("✅ Depth test initialized. Reading every 5 seconds...");
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
