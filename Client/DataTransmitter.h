#pragma once
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") 

class DataTransmitter {
private:
    SOCKET socketFD;
    WSADATA wsaData;

public:
    DataTransmitter();
    ~DataTransmitter();

    void connectToServer(const std::string& ip, int port);
    void send(const std::string& data) const;
    std::string receive() const;
};
