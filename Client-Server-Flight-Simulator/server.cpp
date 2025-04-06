#include "Server.h"
#include "DataParser.h"
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

Server::Server() : serverSocket(INVALID_SOCKET), running(false) {
    // ≥ı ºªØWinsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        throw std::runtime_error("Failed to initialize Winsock");
    }
}

Server::~Server() {
    stop();
    WSACleanup();
}

void Server::start(int port) {
    this->port = port;
    running = true;

    std::cout << "Server starting on port: " << port << std::endl;

    // Create sockets
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        running = false;
        return;
    }

    // Set up address multiplexing
    int optval = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed: " << WSAGetLastError() << std::endl;
    }

    // Binding address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        running = false;
        return;
    }

    // Start listening
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        running = false;
        return;
    }

    std::cout << "Server is listening for clients..." << std::endl;

    // Start accepting connections
    acceptConnections();
}

void Server::stop() {
    running = false;

    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }

    std::cout << "Server stopped." << std::endl;
}

void Server::acceptConnections() {
    while (running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        // Set the timeout
        timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout to allow periodic checking of the running flag
        timeout.tv_usec = 0;

        // Waiting for connection
        int result = select(0, &readSet, nullptr, nullptr, &timeout);

        if (result == SOCKET_ERROR) {
            std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (result > 0 && FD_ISSET(serverSocket, &readSet)) {
            sockaddr_in clientAddr{};
            int clientAddrSize = sizeof(clientAddr);

            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            char clientIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);
            std::cout << "Client connected: " << clientIp << std::endl;

            // Create a new thread to handle client connections
            std::thread([this, clientSocket]() {
                this->handleClient(clientSocket);
                }).detach();
        }
    }
}

void Server::handleClient(int clientSocket) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    bool connectionActive = true;
    int aircraftId = -1;

    // Receive an initialization packet to get the aircraft ID
    int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive initialization data.\n";
        closesocket(clientSocket);
        return;
    }

    // Parsing the initial packet to obtain the aircraft ID
    std::vector<char> initPacket(buffer, buffer + bytesRead);
    auto initData = DataParser::parseTelemPacket(initPacket);
    aircraftId = initData.first;

    if (aircraftId <= 0) {
        std::cerr << "Invalid aircraft ID received.\n";
        closesocket(clientSocket);
        return;
    }

    std::cout << "Received connection from aircraft ID: " << aircraftId << std::endl;

    // Get or create the Flight object associated with this aircraft ID
    Flight& flight = getOrCreateFlight(aircraftId);
    flight.setStartTime(); // Setting the flight start time

    // Receive and process telemetry data sent by the client
    while (connectionActive) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

        // Set the timeout
        timeval timeout;
        timeout.tv_sec = 5;  
        timeout.tv_usec = 0;

        int result = select(0, &readSet, nullptr, nullptr, &timeout);

        if (result == SOCKET_ERROR) {
            std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
            connectionActive = false;
            break;
        }

        if (result == 0) {
            // Timeout, check if the connection is still active
            continue;
        }

        if (FD_ISSET(clientSocket, &readSet)) {
            bytesRead = recv(clientSocket, buffer, bufferSize, 0);
            if (bytesRead <= 0) {
                connectionActive = false;
                break;
            }

            // Parsing telemetry packets
            std::vector<char> dataPacket(buffer, buffer + bytesRead);
            auto telemetryData = DataParser::parseTelemPacket(dataPacket);

            if (telemetryData.first == aircraftId) {
                // Get time and fuel level
                std::string timeStr = telemetryData.second.first;
                double fuelRemaining = telemetryData.second.second;

                // Add fuel data
                flight.addFuelData(fuelRemaining);

                std::cout << "Received data from aircraft " << aircraftId
                    << ", time: " << timeStr
                    << ", fuel: " << fuelRemaining << std::endl;
            }
        }
    }

    // End of flight, set end time, calculate fuel consumption and store data
    flight.setEndTime();
    flight.calculateFuelConsumption();
    storeFlightData(flight);

    std::cout << "Connection with aircraft " << aircraftId << " ended." << std::endl;
    closesocket(clientSocket);
}

Flight& Server::getOrCreateFlight(int aircraftId) {
    std::lock_guard<std::mutex> lock(flightsMutex);

    // Find Aircraft ID
    auto it = activeFlights.find(aircraftId);

    // If it doesn't exist, create a new Flight object
    if (it == activeFlights.end()) {
        auto result = activeFlights.insert(std::make_pair(aircraftId, Flight(aircraftId)));
        it = result.first;
    }

    return it->second;
}

void Server::storeFlightData(const Flight& flight) {
    std::lock_guard<std::mutex> lock(flightsMutex);

    // Save flight data to file
    std::string filename = "Flight_ID_" + std::to_string(flight.getFlightID()) + ".txt";
    flight.saveData(filename);

    std::cout << "Flight stored: " << flight.getFlightDetails() << std::endl;
}

std::pair<int, std::pair<std::string, double>> Server::parseTelemPacket(const std::vector<char>& packet) {
    return DataParser::parseTelemPacket(packet);
}