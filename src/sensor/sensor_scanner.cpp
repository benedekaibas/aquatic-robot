#include <iostream>
#include <vector>


struct Ports {
    std::string sensor;
    std::string port;
};



auto read_ports() {
    std::vector<Ports> ports = {
        {"ph", "/dev/ttyUSB0"},
        {"od", "/dev/ttyUSB1"}
        
    };

    for(const auto& p: ports){
        std::cout << p.sensor << "" << p.port << std::endl;
    }
}

int main() {
    read_ports();
}

