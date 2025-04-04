#include "Server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    int port = 5000;
    if (argc >= 2) {
        port = std::stoi(argv[1]);
    }

    Server server;
    server.start(port);

    return 0;
}
