#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>

MS5837 sensor;

// Constants for depth control
#define CM_CONVERT_FRESHWATER 0.9778 // Conversion constant for freshwater in cm
float desired_depth = 2.0; // Target depth in meters
float lower_level = 0.3; // Minimum allowable error margin in meters
float loop_count = 0

// Variables for depth and temperature
float depth = 0.0, temperatureC = 0.0, temperatureF = 0.0, pressure_hpa = 0.0;
int depthOffset = 0; // Calibration offset for depth

// Motor control parameters
int ST1_S2 = 8; // Pin connected to Sabertooth Motor Controller for Motor 3
float Kp = 30.0; // Proportional gain for motor control

// Setup for Serial, LCD, and Sabertooth communication
const int SerialBaudRate = 9600;
const int LCD_I2C_Addr = 0x27; // I2C address for the LCD Screen
SoftwareSerial SWSerial(NOT_A_PIN, 10); // Serial communication (TX on pin 10)
SabertoothSimplified ST(SWSerial); // Sabertooth controller object

// Initialize Motor Control Function
void setMotor(int motorNum, int power) {
  if (motorNum == 3) {
    digitalWrite(ST1_S2, HIGH);
    ST.motor(1, power); // Motor 3 control on channel 1
    delayMicroseconds(50);
    digitalWrite(ST1_S2, LOW);
  }
}

void setup() {
  Serial.println("Desired depth: ");
  Serial.println(desired_depth);
  Serial.println("Loop count: ");
  Serial.println(loop_count);
  // Initialize pins and serial communication
  pinMode(ST1_S2, OUTPUT);
  SWSerial.begin(SerialBaudRate);
  Serial.begin(SerialBaudRate);
  Wire.begin();

  // Sensor setup
  while (!sensor.init()) {
    Serial.println("Sensor init failed! Retrying in 5 seconds...");
    delay(5000);
  }
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997); // Freshwater density in kg/m³

}

void loop() {
  // Read sensor data
  sensor.read();
  temperatureC = sensor.temperature();
  temperatureF = temperatureC * 9 / 5 + 32;
  pressure_hpa = sensor.pressure();
  
  // Display temperature and pressure
  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.println(" °F");
  
  Serial.print("Pressure: ");
  Serial.print(pressure_hpa, 3);
  Serial.println(" hPa");

  // Calculate current depth
  depth = sensor.depth() + depthOffset;
  Serial.print("Depth: ");
  Serial.print(depth);
  Serial.println(" m");

  // Compute depth error
  float depth_error = abs(desired_depth - depth);

  // Motor control to reach the target depth within acceptable range
  while (depth_error > lower_level) {
    Serial.print("Depth error: ");
    Serial.println(depth_error);

    // Adjust motor power based on depth error
    if (depth < desired_depth && depth < 100) {
      Serial.println("Descending...");
      setMotor(3, depth_error < 0.5 ? 60 : 110); // Lower power when closer to target
    } else if (depth > desired_depth && depth < 100) {
      Serial.println("Ascending...");
      setMotor(3, depth_error < 0.5 ? -80 : -127); // Lower power when closer to target
    }
    delay(10000);

    // Update sensor data within the loop to continually assess depth
    sensor.read();
    depth = sensor.depth() + depthOffset;
    depth_error = abs(desired_depth - depth);

    // Print updated depth
    Serial.print("Current Depth: ");
    Serial.print(depth);
    Serial.println(" m");
    //delay(10000); // Delay for stability in control response
  }

  // Turn off motor and maintain current depth
  Serial.println("Target depth achieved. Holding position...");
  setMotor(3, 0); // Stop motor

  // Wait for a while at the target depth
  Serial.println("Holding depth for 30 seconds...");
  delay(30000); // 30 seconds delay
  desired_depth += 1;
  loop_count += 1;
}