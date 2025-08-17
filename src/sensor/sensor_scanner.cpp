#include <iostream>
#include <vector>
#include <fcntl.h>
#include <errno.h>
//#include <termios.h>
//#include <unistd.h> 

struct Ports {
    const char* sensor;
    const char* port;
};

auto get_ports() {
    Ports ports;

    ports.sensor = "ph";
    ports.port = "/dev/ttyUSB0";

    std::vector<const char*> vec_port = {ports.sensor, ports.port};

    for (const char* p: vec_port) {
        int serial_port;
        const char* serial_port_ptr = ports.sensor;
        serial_port = open(serial_port_ptr, O_RDWR);
        std::cout << serial_port;
    }
}

/*
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
*/

int main() {
    get_ports();
}

