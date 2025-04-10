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

    static std::pair<int, std::pair<int64_t, double>>
        parseTelemPacket(const char* data, size_t length);

    // Retain original function for compatibility
    void extractData();
    std::vector<double> parseFuelData() const;
    std::vector<std::string> parseTimeData() const;

private:
    std::vector<std::string> rawData;
};

#endif