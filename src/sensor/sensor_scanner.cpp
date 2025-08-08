#include <Wire.h>

/*
   Code is responsible for scanning I2C sensors to make sure that
   the connection is possible between the computer and the sensor.

   The main goal is to make sure that the code reads the right port
   where the given sensor is located.
*/

void sensor_setup() {

  Serial.begin (115200);
  int count;

  while(!Serial) {
    delay(100);
  }

  Serial.println("I2C scanning in process...");

  Wire.begin();

  for(int count = 1; i < 120; ++1) {
    Wire.beginTransmission(i);

    if(Wire.endTransmission() == 0) {
      Serial.println("Found address: ");
      Serial.println(i, DEC);
      count++;
    }
  }
  Serial.println("Found");
  Serial.println((count, DEC))
}

void loop(){}
