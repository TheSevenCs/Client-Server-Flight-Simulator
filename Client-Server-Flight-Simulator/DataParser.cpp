#define _CRT_SECURE_NO_WARNINGS
#include "DataParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

// Retain original constructor for compatibility
DataParser::DataParser(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        rawData.push_back(line);
    }
}

// Parsing one-line telemetry data
std::pair<std::string, double> DataParser::parseTelemDataLine(const std::string& line) {
    // Processing format: ¡°FUEL TOTAL QUANTITY,12_3_2023 14:56:47,47.865124¡±
    size_t firstComma = line.find(',');
    if (firstComma == std::string::npos) {
        return { "", 0.0 };
    }

    size_t secondComma = line.find(',', firstComma + 1);
    if (secondComma == std::string::npos) {
        return { "", 0.0 };
    }

    std::string timestamp = line.substr(firstComma + 1, secondComma - firstComma - 1);
    std::string fuelStr = line.substr(secondComma + 1);

    // Clean up spaces and extra commas in strings
    fuelStr.erase(std::remove(fuelStr.begin(), fuelStr.end(), ' '), fuelStr.end());
    fuelStr.erase(std::remove(fuelStr.begin(), fuelStr.end(), ','), fuelStr.end());

    double fuel = 0.0;
    try {
        fuel = std::stod(fuelStr);
    }
    catch (...) {
        std::cerr << "Failed to parse fuel value: " << fuelStr << std::endl;
    }

    return { timestamp, fuel };
}

// Parsing network packets
std::pair<int, std::pair<std::string, double>> DataParser::parseTelemPacket(const std::vector<char>& packet) {
    // Assume packet format is: [4-byte aircraft ID][8-byte timestamp][8-byte fuel amount]
    if (packet.size() < sizeof(int) + sizeof(int64_t) + sizeof(double)) {
        return { -1, {"", 0.0} }; // Invalid packet
    }

    int offset = 0;

    // Extract aircraft ID
    int aircraftId;
    std::memcpy(&aircraftId, packet.data() + offset, sizeof(aircraftId));
    offset += sizeof(aircraftId);

    // Extract timestamps
    int64_t epochMillis;
    std::memcpy(&epochMillis, packet.data() + offset, sizeof(epochMillis));
    offset += sizeof(epochMillis);

    // Convert timestamps to a readable format
    auto timePoint = std::chrono::system_clock::time_point(std::chrono::milliseconds(epochMillis));
    auto timeT = std::chrono::system_clock::to_time_t(timePoint);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    std::string timeStr = ss.str();

    // Fuel extraction
    double fuelRemaining;
    std::memcpy(&fuelRemaining, packet.data() + offset, sizeof(fuelRemaining));

    return { aircraftId, {timeStr, fuelRemaining} };
}

// Keep original functions for compatibility
void DataParser::extractData() {
    for (const auto& line : rawData) {
        std::cout << "Extracting: " << line << std::endl;
    }
}

// Parsing of fuel data (modified to handle new format)£©
std::vector<double> DataParser::parseFuelData() const {
    std::vector<double> fuelValues;

    for (const auto& line : rawData) {
        auto parsedData = parseTelemDataLine(line);
        if (parsedData.second > 0) {
            fuelValues.push_back(parsedData.second);
        }
    }

    return fuelValues;
}

// Parsing time data (modified to handle new format)
std::vector<std::string> DataParser::parseTimeData() const {
    std::vector<std::string> timeValues;

    for (const auto& line : rawData) {
        auto parsedData = parseTelemDataLine(line);
        if (!parsedData.first.empty()) {
            timeValues.push_back(parsedData.first);
        }
    }

    return timeValues;
}