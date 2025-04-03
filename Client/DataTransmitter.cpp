#include "DataTransmitter.h"
#include <iostream>

DataTransmitter::DataTransmitter() : socketFD(INVALID_SOCKET) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
    }
}

DataTransmitter::~DataTransmitter() {
    if (socketFD != INVALID_SOCKET) {
        closesocket(socketFD);
    }
    WSACleanup();
}

void DataTransmitter::connectToServer(const std::string& ip, int port) {
    socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(socketFD, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
    }
    else {
        std::cout << "Connected to server at " << ip << ":" << port << std::endl;
    }
}

void DataTransmitter::send(const std::string& data) const {
    if (socketFD != INVALID_SOCKET) {
        ::send(socketFD, data.c_str(), static_cast<int>(data.size()), 0);
    }
}

std::string DataTransmitter::receive() const {
    char buffer[1024] = { 0 };
    int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        return std::string(buffer, bytesReceived);
    }
    return {};
}
