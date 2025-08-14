"""Receiving and processing data from Atlas Scientific sensors."""
import time
import serial

#from serial import SerialException, SerialTimeoutException



def store_ports():
    """Storing sensor ports."""
    ports_list = []

    ph = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    ports_list.append(ph)
    do = serial.Serial('/dev/ttyUSB1', 9600, timeout=1)
    ports_list.append(do)
    orp = serial.Serial('/dev/ttyUSB2', 9600, timeout=1)
    ports_list.append(orp)
    ec = serial.Serial('/dev/ttyUSB3', 9600, timeout=1)
    ports_list.append(ec)
    rtd = serial.Serial('/dev/ttyUSB4', 9600, timeout=1)
    ports_list.append(rtd)

    return ports_list

def one_port_read():
    """This function can be used to test different ports."""

    dh = serial.Serial('dev/ttyUSB1', 9600, timeout=1)
    dh.write(b"R\r")

    time.sleep(1)
    while dh.in_waiting:
        data = dh.readline().decode().split()
        print(f"{dh.port}: {data}")

def single_read():
    """Get a single reading from the sensor to see if connection was successful."""
    ports = store_ports()
    serials = []

    for port in ports:
        try:
            serials.append(port)
        except serial.SerialException as s:
            print(f"Could not find or connect to port: {port}: {s}")

    while True:
        for srl in range(len(serials)): # check this line if it resolves our problem with the iteration
            srl_char = serials[srl].read(1)

            try:
                srl_char = srl_char.readline().decode().split()
            except UnicodeDecodeError:
                pass


if __name__ == "__main__":
    single_read()
