#pragma once
#include <chrono>
#include <vector>
#include <string>

class TelemetryData {
private:
    std::chrono::system_clock::time_point timestamp;
    double fuelRemaining;
    int aircraftID;
    std::vector<char> packetizedData;

public:
    TelemetryData();
    TelemetryData(std::chrono::system_clock::time_point timestamp, double fuelRemaining, int aircraftID);

    std::chrono::system_clock::time_point getTimestamp() const;
    double getFuelRemaining() const;
    int getAircraftID() const;

    bool packetize();
    const std::vector<char>& getPacketizedData() const;
    static TelemetryData depacketize(const std::vector<char>& data);

    std::string toString() const;
};