#include "SocketDataTransmitter.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

SocketDataTransmitter::SocketDataTransmitter() : socket(INVALID_SOCKET), connected(false) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

SocketDataTransmitter::~SocketDataTransmitter() {
    disconnect();
    WSACleanup();
}

bool SocketDataTransmitter::connect(const std::string& serverIP, int port) {
    if (connected) {
        disconnect();
    }

    // Create socket
    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    int result = ::connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(socket);
        socket = INVALID_SOCKET;
        return false;
    }

    connected = true;
    return true;
}

void SocketDataTransmitter::disconnect() {
    if (socket != INVALID_SOCKET) {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
    connected = false;
}

bool SocketDataTransmitter::send(const std::vector<char>& data) {
    if (!connected || data.empty()) {
        return false;
    }

    // Send data size first
    int dataSize = static_cast<int>(data.size());
    int bytesSent = ::send(socket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    if (bytesSent != sizeof(dataSize)) {
        std::cerr << "Failed to send data size: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Send actual data
    bytesSent = ::send(socket, data.data(), dataSize, 0);
    if (bytesSent != dataSize) {
        std::cerr << "Failed to send all data: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

bool SocketDataTransmitter::receive(std::vector<char>& data) {
    if (!connected) {
        return false;
    }

    // Receive data size first
    int dataSize = 0;
    int bytesReceived = ::recv(socket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    if (bytesReceived != sizeof(dataSize) || dataSize <= 0) {
        std::cerr << "Failed to receive data size: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Resize buffer and receive data
    data.resize(dataSize);
    bytesReceived = ::recv(socket, data.data(), dataSize, 0);
    if (bytesReceived != dataSize) {
        std::cerr << "Failed to receive all data: " << WSAGetLastError() << std::endl;
        data.clear();
        return false;
    }

    return true;
}

bool SocketDataTransmitter::isConnected() const {
    return connected;
}