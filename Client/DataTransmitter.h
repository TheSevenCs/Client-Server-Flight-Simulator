#pragma once
#include <vector>
#include <string>

class DataTransmitter {
public:
    virtual ~DataTransmitter() = default;
    virtual bool connect(const std::string& serverIP, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool send(const std::vector<char>& data) = 0;
    virtual bool receive(std::vector<char>& data) = 0;
    virtual bool isConnected() const = 0;
};