#include "DataParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

// This constructor reads the telemetry file and stores its lines in rawData
DataParser::DataParser(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        rawData.push_back(line);
    }
}

//The function to extract data by displaying each stored line 
void DataParser::extractData() {
    for (const auto& line : rawData) {
        std::cout << "Extracting: " << line << std::endl;
    }
}

// Parses fuel data from the telemetry file, extracting only fuel values
std::vector<double> DataParser::parseFuelData() const {
    std::vector<double> fuelValues;

    for (const auto& line : rawData) {
        std::istringstream iss(line);
        std::string time;
        double fuel;

        if (iss >> time >> fuel) {
            fuelValues.push_back(fuel);
        }
    }

    return fuelValues;
}

// Parses time data from the telemetry file, extracting only timestamps
std::vector<std::string> DataParser::parseTimeData() const {
    std::vector<std::string> timeValues;

    for (const auto& line : rawData) {
        std::istringstream iss(line);
        std::string time;
        double fuel;

        if (iss >> time >> fuel) {
            timeValues.push_back(time);
        }
    }

    return timeValues;
}
