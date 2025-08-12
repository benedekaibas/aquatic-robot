# Documentation file for the sensor_scanner.py file.

## Setup sensors

In order to get the readings from the Atlas Scientific sensors we have to connect the sensors on the robot
to the box that contains the EZO circuits. There are two different robots (yellow & red) with two boxes
containing the circuits (yellow & red). To get the right readings, you have to connect the yellow box
with the yellow robot and do the same with the other robot as well.

The cords are labeled by colors and you have to match the right colors on the cord with the EZO circuits
inside the box. (Open the box to see which circuit has which color).

Once, the sensors are connected with the box, connect the box to the computer to be able to get the readings.

## Usage of the scanner file

To be able getting the readings out of the sensors we have the `sensor_scanner.py` file which is responsible for
getting the readings out of the sensors through the EZO circuits. To run this file you have to run the following
command in the terminal `python sensor_scanner.py`. Once you run the command you should be able to see the readings
of the sensor.

## Sensors and their addresses

Each sensor has a different hexadecimal address. We should know each sensor's address in order to get access to them and
also when we need to troubleshoot this information can be helpful. Below you can read each sensor's address in hexadecimal
value. Also if you take a look at the `sensor_scanner.py` file you can see how to addresses are handled/processed.

- Addresses of the sensors:

- 1) pH: 0x63
- 2) Dissolved Oxygen: 0x61
- 3) ORP: 0x62
- 4) EC: 0x64
- 5) RTD: 0x68

## Error messages

The `sensor_scanner` file is written in a way to handle errors in a proper way. In some cases it gives error messages
to the user which can help in the troubleshooting.

### Could not connect to the sensor...

This error happens if the computer is not connected to the box. Make sure to check the usb cable and to connect it to the
right port on the computer.

