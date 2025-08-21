# Documentation for Atlas Scientific Sensors

## What do our sensor do?

We use Atlas Scientific sensors that can measure pH, ORP, conductivity, dissolved oxygen, temperature, and pressure.
While our robot moves in different depth levels we allow the sensors to take readings at multiple levels in the water
and return readings, so we can see differences based on depth levels.

This documentation is for people who are working on the AquaGator project to guide them how to setup, maintain, operate, and take
readings from the sensors.

## Setup

To setup the sensors you will need to attach them to one of the boxes (Yellow robot goes with the yellow sensors' box and the red robot has to
be attached to the red sensors' box). To attach the sensors' box to a laptop a USB cable is needed that goes out from the sensors' box.

While setting up the sensors make sure to leave them for 5-10 minutes outside of water and start the Atlas Scientific desktop. This is important
step since we need the sensors to calibrate themselves. If the sensors go straight to the water then the readings will be off.

When placed in water we also have to wait 5-10 minutes to get the right readings. After that we can start recording the readings.

Some of the sensors come with small covers. It is important to take them off before testing and put them back after testing. The sensors that come with a cover do need humidity while storing them, so make sure to put tap water into the covers after done with testing.

Important note: do not put too much water into the covers because the water should not touch the sensors. Just make sure there is some water in the bottom of the cover to provide humid environment for the sensors.

## Sensors' result

To see sensor results you have three options: use AtlasDesktop, run the `sensor_scanner.py` or run the `sensor_scanner.cpp`

### AtlasDesktop

To most convenient way of getting all the results is opening that AtlasDesktop monitoring system on the Windows laptop or on our tablet. This will display all the sensors' readings once you do the necessary setup that is explained in the `Setup` section. Once the setup is done and the box is attached to the computer you can see the readings. NOTE: Remember to wait until 5-10 minutes, as explained in the `Setup` section, for the readings to stabilize.

### sensor_scanner.py

This file is written, so users can compare the sensors' results from the AtlasDestop with the output of the `sensor_scanner.py` file. To learn more about how the `sensor_scanner.py` file works visit its documentation.

### sensor_scanner.cpp

This version of getting output straight from the sensor can be helpful for analyzing where dissolved oxygen changes in the water, so we can use the data through the `sensor_scanner.cpp` file in the `robot_motion.ino` file and the robot can stay at the level where significant dissolved oxygen change happens in the water.

Use this file for the process described above!

