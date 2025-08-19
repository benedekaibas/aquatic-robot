"""Reading dissolved oxygen sensor's value through serial port."""
import time
import serial

#from serial import SerialException, SerialTimeoutException



def do_port():
    """Storing dissolved oxygen sensor's value in a list."""
    do_port = []

    do_value = serial.Serial('/dev/ttyUSB1', 9600, timeout=1)
    do_port.append(do_value)

    return do_port

def read_do_port():
    """Get readings from the do sensor."""
    do_value = do_port()

    while True:
        for do in do_value:
            do.write(b"R\r")
            time.sleep(0.03)

            if do.in_waiting > 0:
                data = do.readline().decode().split()
                print(f"Do {do} level: ", {data})
            time.sleep(0.01)

if __name__ == "__main__":
    read_do_port()

