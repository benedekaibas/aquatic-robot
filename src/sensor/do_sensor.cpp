#include <iostream>
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

int open_serial_port() {
  int serial_port = open("/dev/ttyUSB1", O_RDWR);

  if(serial_port < 0) {
    std::cout << "Error %i from open: %s\n", errno, strerror(errno);
  }
  return serial_port;
}

int main() {
  open_serial_port();
}
