import serial
import time

from serial import SerialException, SerialTimeoutException

ports = {
            "ph": 'dev/ttyUSB0',
            "do": 'dev/ttyUSB1',
            "orp": 'dev/ttyUSB2',
            "ec": 'dev/ttyUSB3',
            "rtd": 'dev/ttyUSB4'

            }

def store_ports():
    ports_list = []

    ph = serial.Serial('dev/ttyUSB0', 9600, timeout=1)
    ports_list.append(ph)
    do = serial.Serial('dev/ttyUSB1', 9600, timeout=1)
    ports_list.append(do)
    orp = serial.Serial('dev/ttyUSB2', 9600, timeout=1)
    ports_list.append(orp)
    ec = serial.Serial('dev/ttyUSB3', 9600, timeout=1)
    ports_list.append(ec)
    rtd = serial.Serial('dev/ttyUSB4', 9600, timeout=1)
    ports_list.append(rtd)

    return ports_list

def single_read():
    """Get a single reading from the sensor to see if connection was successful."""
    ports = store_ports()
    for port in ports:
        port.write(b"R\r")

    time.sleep(1) # time sleep for not overwhelming the sensor, but it might need to remove
    while(port.in_waiting):
        print(port.readline().decode().split())

    port.close()


if __name__ == "__main__":
    single_read()
