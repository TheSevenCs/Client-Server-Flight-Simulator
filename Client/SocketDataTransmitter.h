#pragma once
#include "DataTransmitter.h"
#include <WinSock2.h>

class SocketDataTransmitter : public DataTransmitter {
private:
    SOCKET socket;
    bool connected;

public:
    SocketDataTransmitter();
    ~SocketDataTransmitter() override;

    bool connect(const std::string& serverIP, int port) override;
    void disconnect() override;
    bool send(const std::vector<char>& data) override;
    bool receive(std::vector<char>& data) override;
    bool isConnected() const override;
};