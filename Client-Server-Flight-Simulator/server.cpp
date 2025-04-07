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

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        running = false;
        return;
    }

    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "Server is listening for clients..." << std::endl;
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

        timeval timeout{};
        timeout.tv_sec = 1;

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

            std::thread([this, clientSocket]() {
                this->handleClient(clientSocket);
                }).detach();
        }
    }
}
std::chrono::system_clock::time_point parseTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // Adjust format as needed
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse time string");
    }
    std::time_t timeT = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(timeT);
}


void Server::handleClient(int clientSocket) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    bool connectionActive = true;
    int aircraftId = idManager.generateID();
    bool firstPacket = true;

    // Send ID to client
    std::string idMessage = std::to_string(aircraftId) + "\n";
    send(clientSocket, idMessage.c_str(), static_cast<int>(idMessage.size()), 0);
    std::cout << "Assigned and sent aircraft ID: " << aircraftId << "\n";

    Flight& flight = getOrCreateFlight(aircraftId);
    std::string lastTimeStr;

    while (connectionActive) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

        timeval timeout{};
        timeout.tv_sec = 5;

        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        if (result == SOCKET_ERROR) {
            std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (result > 0 && FD_ISSET(clientSocket, &readSet)) {
            int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
            if (bytesRead <= 0) {
                break;
            }

            std::vector<char> dataPacket(buffer, buffer + bytesRead);
            auto telemetryData = DataParser::parseTelemPacket(dataPacket);

            if (telemetryData.first == aircraftId) {
                std::string timeStr = telemetryData.second.first;
                double fuel = telemetryData.second.second;

                if (firstPacket) {
                    flight.setStartTime(parseTime(timeStr));
                    firstPacket = false;
                }

                lastTimeStr = timeStr;
                flight.addFuelData(fuel);

                std::cout << "Aircraft " << aircraftId << ": time=" << timeStr << ", fuel=" << fuel << std::endl;
            }
        }
    }

    if (!lastTimeStr.empty()) {
        flight.setEndTime(parseTime(lastTimeStr));
    }

    flight.calculateFuelConsumption();
    storeFlightData(flight);

    std::cout << "Connection with aircraft " << aircraftId << " ended.\n";
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