#include <iostream>
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

void open_serial_port() {
  std::string port = "/dev/ttyUSB1";
  int serial_port = open(port.c_str(), O_RDWR);

  if(serial_port < 0) {
    std::cerr << "Error while opning the port: " << strerror(errno);
  } else {
    std::cout << "Opened Serial Port" << serial_port;
  }

  char read_buffer[256];
  std::string response;


  do {
    int num_bytes = read(serial_port, &read_buffer, sizeof(read_buffer));

    if(num_bytes > 0) {
      response += std::string(read_buffer);
      std::cout << read_buffer;
    }

  } while(read_buffer[0] != 'X');

  std::cout << "Response: " << "\n";
  std::cout << response;

  close(serial_port);
}

int main() {
  open_serial_port();
}
