#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

static bool configure_port(int fd, speed_t baud = B9600) {
    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "tcgetattr failed: " << strerror(errno) << "\n";
        return false;
    }


    cfmakeraw(&tty);

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag = 0;
    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
    tty.c_cflag |= (CS8 | CREAD | CLOCAL);
    tty.c_lflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;

    if (cfsetispeed(&tty, baud) != 0 || cfsetospeed(&tty, baud) != 0) {
        std::cerr << "cfset[io]speed failed: " << strerror(errno) << "\n";
        return false;
    }

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "tcsetattr failed: " << strerror(errno) << "\n";
        return false;
    }

    if (tcflush(fd, TCIFLUSH) != 0) {
        std::cerr << "tcflush failed: " << strerror(errno) << "\n";
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    const char* dev = (argc > 1) ? argv[1] : "/dev/ttyUSB0";

    int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "open(" << dev << ") failed: " << strerror(errno) << "\n";
        return 1;
    }

    if (!configure_port(fd, B9600)) {
        close(fd);
        return 1;
    }

    std::cout << "Reading from " << dev << " at 9600 8N1 (timeout 1s)...\n";

    std::string line;
    std::vector<char> buf(256);

    while (true) {
        ssize_t n = read(fd, buf.data(), buf.size());
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "read failed: " << strerror(errno) << "\n";
            break;
        }
        if (n == 0) {
            continue;
        }

        for (ssize_t i = 0; i < n; ++i) {
            char c = buf[i];
            if (c == '\r') continue;
            if (c == '\n') {
                if (!line.empty()) {
                    std::cout << "RX: " << line << "\n";
                    line.clear();
                }
            } else {
                line.push_back(c);
            }
        }
    }

    close(fd);
    return 0;
}

