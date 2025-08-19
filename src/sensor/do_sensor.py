"""Reading dissolved oxygen sensor's value through serial port."""
import time
import serial

#from serial import SerialException, SerialTimeoutException



def read_do_port():
    """Read dissolved oxygen value."""
    do_value = serial.Serial('/dev/ttyUSB1', 9600, timeout=1)

    do_value.write(b"R\r")
    time.sleep(0.03)
    count = 0

    while(count < 10):
        do_value.write(b"R\r")
        count += 1
        data = do_value.readline().decode().split()
        print(f"{do_value}: {data}")


if __name__ == "__main__":
    read_do_port()

