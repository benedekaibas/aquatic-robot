import serial
import time

srl = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
srl.write(b"R\r")

time.sleep(1)

while(srl.in_waiting):
    print(srl.readline().decode().split())

srl.close()

