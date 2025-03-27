#define _CRT_SECURE_NO_WARNINGS

#include "TelemetryData.h"
#include <sstream>
#include <iomanip>

TelemetryData::TelemetryData()
    : timestamp(std::chrono::system_clock::now()), fuelRemaining(0.0), aircraftID(-1) {
}

TelemetryData::TelemetryData(std::chrono::system_clock::time_point timestamp, double fuelRemaining, int aircraftID)
    : timestamp(timestamp), fuelRemaining(fuelRemaining), aircraftID(aircraftID) {
}

std::chrono::system_clock::time_point TelemetryData::getTimestamp() const {
    return timestamp;
}

double TelemetryData::getFuelRemaining() const {
    return fuelRemaining;
}

int TelemetryData::getAircraftID() const {
    return aircraftID;
}

bool TelemetryData::packetize() {
    packetizedData.clear();

    // Header - 4 bytes for aircraft ID
    char* idPtr = reinterpret_cast<char*>(&aircraftID);
    packetizedData.insert(packetizedData.end(), idPtr, idPtr + sizeof(aircraftID));

    // 8 bytes for timestamp (convert to milliseconds since epoch)
    auto epochMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count();
    char* timePtr = reinterpret_cast<char*>(&epochMillis);
    packetizedData.insert(packetizedData.end(), timePtr, timePtr + sizeof(epochMillis));

    // 8 bytes for fuel remaining
    char* fuelPtr = reinterpret_cast<char*>(&fuelRemaining);
    packetizedData.insert(packetizedData.end(), fuelPtr, fuelPtr + sizeof(fuelRemaining));

    return true;
}

const std::vector<char>& TelemetryData::getPacketizedData() const {
    return packetizedData;
}

TelemetryData TelemetryData::depacketize(const std::vector<char>& data) {
    if (data.size() < sizeof(int) + sizeof(int64_t) + sizeof(double)) {
        return TelemetryData(); // Invalid data
    }

    int offset = 0;

    // Extract aircraft ID
    int aircraftID;
    std::memcpy(&aircraftID, data.data() + offset, sizeof(aircraftID));
    offset += sizeof(aircraftID);

    // Extract timestamp
    int64_t epochMillis;
    std::memcpy(&epochMillis, data.data() + offset, sizeof(epochMillis));
    offset += sizeof(epochMillis);

    std::chrono::system_clock::time_point timestamp =
        std::chrono::system_clock::time_point(std::chrono::milliseconds(epochMillis));

    // Extract fuel remaining
    double fuelRemaining;
    std::memcpy(&fuelRemaining, data.data() + offset, sizeof(fuelRemaining));

    return TelemetryData(timestamp, fuelRemaining, aircraftID);
}

std::string TelemetryData::toString() const {
    auto timeT = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << "Time: " << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S")
        << ", Fuel: " << fuelRemaining
        << ", Aircraft ID: " << aircraftID;
    return ss.str();
}