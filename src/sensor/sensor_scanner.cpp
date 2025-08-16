#include <iostream>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h> 

struct Ports {
    const char* sensor;
    const char* port;
};


auto read_ports() {
    std::vector<Ports> ports = {
        {"ph", "/dev/ttyUSB0"},
        {"od", "/dev/ttyUSB1"}
        
    };

    for(const char* p: ports){
       int serial_port;
       const char* serial_port_ptr = &p.port;
       serial_port = open(serial_port_ptr, O_RDWR);
    }
}

int main() {
    read_ports();
}

