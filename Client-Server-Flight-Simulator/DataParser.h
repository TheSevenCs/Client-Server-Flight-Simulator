#pragma once
#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <string>
#include <vector>
#include <utility>

class DataParser {
public:
    // Constructor to read telemetry data from a file (reserved for compatibility)
    DataParser(const std::string& filename);

    // Parsing individual telemetry data rows
    static std::pair<std::string, double> parseTelemDataLine(const std::string& line);

    // Extract telemetry data from binary packets
    static std::pair<int, std::pair<std::string, double>> parseTelemPacket(const std::vector<char>& packet);

    // Retain original function for compatibility
    void extractData();
    std::vector<double> parseFuelData() const;
    std::vector<std::string> parseTimeData() const;

private:
    std::vector<std::string> rawData;
};

#endif