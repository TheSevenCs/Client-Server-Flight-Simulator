#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "SocketDataTransmitter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <ctime>

Client::Client(const std::string& serverIP, int serverPort)
    : serverIP(serverIP), serverPort(serverPort), isConnected(false) {

  
    transmitter = std::make_unique<SocketDataTransmitter>();

    aircraft = Aircraft(0);
}

Client::~Client() {
    disconnectFromServer();
}

bool Client::initialize(const std::string& telemetryFilePath) {
    this->telemetryFilePath = telemetryFilePath;

    // Verify that the file exists
    std::ifstream file(telemetryFilePath);
    if (!file) {
        std::cerr << "Failed to open telemetry file: " << telemetryFilePath << std::endl;
        return false;
    }

  
    std::string line;
    if (std::getline(file, line)) {
        // file size
        size_t firstComma = line.find(',');
        if (firstComma != std::string::npos) {
            size_t secondComma = line.find(',', firstComma + 1);
            if (secondComma != std::string::npos) {
                std::string fuelStr = line.substr(secondComma + 1);
                fuelStr.erase(std::remove_if(fuelStr.begin(), fuelStr.end(),
                    [](char c) { return c == ' ' || c == ',' || c == '\r' || c == '\n'; }), fuelStr.end());
                try {
                    double initialFuel = std::stod(fuelStr);
                    aircraft.setInitialFuel(initialFuel);
                }
                catch (...) {
                    std::cerr << "Failed to parse initial fuel value." << std::endl;
                }
            }
        }
    }

    file.close();
    return true;
}

//bool Client::connectToServer() {
//    if (isConnected) {
//        return true;
//    }
//
//    isConnected = transmitter->connect(serverIP, serverPort);
//
//    if (isConnected) {
//        // Creating the initial packet
//        auto now = std::chrono::system_clock::now();
//        TelemetryData initialData(now, aircraft.getInitialFuel(), aircraft.getID());
//        initialData.packetize();
//
//        // Send initial pack
//        if (!transmitter->send(initialData.getPacketizedData())) {
//            isConnected = false;
//            return false;
//        }
//    }
//
//    return isConnected;
//}

bool Client::connectToServer() {
    if (isConnected) {
        return true;
    }

    isConnected = transmitter->connect(serverIP, serverPort);

    if (isConnected) {
        // Receive the ID assigned by the server
        std::vector<char> buffer(64, 0); // Create a large enough buffer
        bool idReceived = transmitter->receive(buffer);

        if (!idReceived || buffer.empty()) {
            std::cerr << "Failed to receive ID from server." << std::endl;
            disconnectFromServer();
            return false;
        }

        // Resolve the ID
        std::string idStr(buffer.begin(), buffer.end());
        idStr = idStr.substr(0, idStr.find('\0')); // Remove trailing blank characters

        try {
            int id = std::stoi(idStr);
            // Setting the aircraft ID
            aircraft.setID(id);
            std::cout << "Received ID from server: " << id << std::endl;
        }
        catch (...) {
            std::cerr << "Failed to parse ID: " << idStr << std::endl;
            disconnectFromServer();
            return false;
        }

    }

    return isConnected;
}


void Client::disconnectFromServer() {
    if (transmitter) {
        transmitter->disconnect();
    }
    isConnected = false;
}

bool Client::processFile() {
    if (!isConnected) {
        std::cerr << "Not connected to server." << std::endl;
        return false;
    }

    std::ifstream file(telemetryFilePath);
    if (!file) {
        std::cerr << "Failed to open telemetry file: " << telemetryFilePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove whitespace at the beginning and end of a line
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        // Sample Format: 1_3_2023 12:35:34,33.571247,
        size_t comma = line.find(',');
        if (comma == std::string::npos) continue;

        std::string timeStr = line.substr(0, comma);
        std::string fuelStr = line.substr(comma + 1);

        // Clearing time and fuel strings
        timeStr.erase(0, timeStr.find_first_not_of(" \t\r\n"));
        timeStr.erase(timeStr.find_last_not_of(" \t\r\n") + 1);

        fuelStr.erase(0, fuelStr.find_first_not_of(" \t\r\n"));
        // Remove trailing commas and whitespace
        size_t lastNonComma = fuelStr.find_last_not_of(",\t\r\n ");
        if (lastNonComma != std::string::npos) {
            fuelStr = fuelStr.substr(0, lastNonComma + 1);
        }

        double fuelRemaining = 0.0;
        try {
            fuelRemaining = std::stod(fuelStr);
        }
        catch (...) {
            std::cerr << "Failed to parse fuel value: '" << fuelStr << "'" << std::endl;
            continue; // Skip lines that fail parsing
        }

        // Creating a point in time - parsing a time in "1_3_2023 12:35:34" format
        std::tm tm = {};
        int month, day, year, hour, min, sec;
        if (sscanf(timeStr.c_str(), "%d_%d_%d %d:%d:%d",
            &month, &day, &year, &hour, &min, &sec) == 6) {
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = min;
            tm.tm_sec = sec;

            auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            // Creating Telemetry Data
            TelemetryData data(time_point, fuelRemaining, aircraft.getID());

            // Transfer to server
            if (!transmitTelemetryData(data)) {
                std::cerr << "Failed to transmit telemetry data." << std::endl;
                file.close();
                return false;
            }

            // 50 ms latency to simulate real-time data transfer
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else {
            std::cerr << "Failed to parse time: '" << timeStr << "'" << std::endl;
        }
    }

    file.close();
    std::cout << "File processing completed." << std::endl;
    return true;
}

bool Client::transmitTelemetryData(const TelemetryData& data) {
    if (!isConnected) {
        return false;
    }

    // Serialized data
    TelemetryData packetData = data;
    packetData.packetize();

    // Send data to the server
    return transmitter->send(packetData.getPacketizedData());
}

const Aircraft& Client::getAircraft() const {
    return aircraft;
}

void Client::setAircraft(const Aircraft& aircraft) {
    this->aircraft = aircraft;
}

bool Client::isServerConnected() const {
    return isConnected && transmitter->isConnected();
}
