import serial
import time

from serial import SerialTimeoutException

def connect_sensor():
    try:
        srl = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
        return srl
    except serial.SerialTimeoutException:
        print("Sensor is not responding in time. Serial timeout...")


def single_read():
    """Get a single reading from the sensor to see if connection was successful."""
    srl = connect_sensor()
    srl.write(b"R\r")

    time.sleep(1) # time sleep for not overwhelming the sensor, but it might need to remove
    while(srl.in_waiting):
        print(srl.readline().decode().split())

    srl.close()

def continuous_reading():
    """Receive continuous reading from the sensor."""
    srl = connect_sensor()
    srl.write(b"C,5\r")

    return srl


if __name__ == "__main__":
    res = continuous_reading()
    print(res)
