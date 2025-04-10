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
std::pair<int, std::pair<int64_t, double>>
DataParser::parseTelemPacket(const char* data, size_t length) {
    if (length < sizeof(int) + sizeof(int64_t) + sizeof(double)) {
        return { -1, {0, 0.0} }; // Invalid packet
    }

    int offset = 0;

    int aircraftId;
    std::memcpy(&aircraftId, data + offset, sizeof(aircraftId));
    offset += sizeof(aircraftId);

    int64_t epochMillis;
    std::memcpy(&epochMillis, data + offset, sizeof(epochMillis));
    offset += sizeof(epochMillis);

    double fuelRemaining;
    std::memcpy(&fuelRemaining, data + offset, sizeof(fuelRemaining));

    return { aircraftId, {epochMillis, fuelRemaining} };
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