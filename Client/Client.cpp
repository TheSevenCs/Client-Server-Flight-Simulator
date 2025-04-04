#include "Client.h"
#include "UniqueIDManager.h"
#include "SocketDataTransmitter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>

Client::Client(const std::string& serverIP, int serverPort)
    : serverIP(serverIP), serverPort(serverPort), isConnected(false) {

    // Create a socket data transmitter
    transmitter = std::make_unique<SocketDataTransmitter>();

    // Create an aircraft and assign a unique ID
    int uniqueID = UniqueIDManager::getInstance()->generateUniqueID();
    aircraft = Aircraft(uniqueID);
}

Client::~Client() {
    disconnectFromServer();
}

bool Client::initialize(const std::string& telemetryFilePath) {
    this->telemetryFilePath = telemetryFilePath;

    // Validate file exists
    std::ifstream file(telemetryFilePath);
    if (!file) {
        std::cerr << "Failed to open telemetry file: " << telemetryFilePath << std::endl;
        return false;
    }

    // Read the first line to extract initial fuel if available
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string time;
        double fuel;

        if (iss >> time >> fuel) {
            aircraft.setInitialFuel(fuel);
        }
    }

    file.close();
    return true;
}

bool Client::connectToServer() {
    if (isConnected) {
        return true;
    }

    isConnected = transmitter->connect(serverIP, serverPort);

    if (isConnected) {
        // Send initial connection packet with aircraft ID
        TelemetryData initialData(std::chrono::system_clock::now(),
            aircraft.getInitialFuel(),
            aircraft.getID());
        initialData.packetize();
        transmitter->send(initialData.getPacketizedData());
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
        // Parse line - format: timestamp fuel_remaining
        std::istringstream iss(line);
        std::string timeStr;
        double fuelRemaining;

        if (iss >> timeStr >> fuelRemaining) {
            // Create telemetry data
            auto now = std::chrono::system_clock::now();
            TelemetryData data(now, fuelRemaining, aircraft.getID());

            // Transmit to server
            if (!transmitTelemetryData(data)) {
                std::cerr << "Failed to transmit telemetry data." << std::endl;
                file.close();
                return false;
            }

            // Small delay to simulate real-time data transmission
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

    // Packetize data
    TelemetryData packetData = data;
    packetData.packetize();

    // Send data to server
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

Client::Client(Client&& other) noexcept
    : serverIP(std::move(other.serverIP)),
    serverPort(other.serverPort),
    isConnected(other.isConnected),
    telemetryFilePath(std::move(other.telemetryFilePath)),
    transmitter(std::move(other.transmitter)),
    aircraft(std::move(other.aircraft)) {
}

Client& Client::operator=(Client&& other) noexcept {
    if (this != &other) {
        serverIP = std::move(other.serverIP);
        serverPort = other.serverPort;
        isConnected = other.isConnected;
        telemetryFilePath = std::move(other.telemetryFilePath);
        transmitter = std::move(other.transmitter);
        aircraft = std::move(other.aircraft);
    }
    return *this;
}
