#pragma once
#include "Aircraft.h"
#include "TelemetryData.h"
#include "DataTransmitter.h"
#include <string>
#include <memory>

class Client {
private:
    std::string serverIP;
    int serverPort;
    std::string telemetryFilePath;
    Aircraft aircraft;
    std::unique_ptr<DataTransmitter> transmitter;
    bool isConnected;

public:
    Client(const std::string& serverIP = "127.0.0.1", int serverPort = 8080);
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) noexcept;
    Client& operator=(Client&&) noexcept;

    bool initialize(const std::string& telemetryFilePath);
    bool connectToServer();
    void disconnectFromServer();
    bool processFile();
    bool transmitTelemetryData(const TelemetryData& data);
    const Aircraft& getAircraft() const;
    void setAircraft(const Aircraft& aircraft);
    bool isServerConnected() const;
};