#include "Server.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


std::mutex flightDataMutex;

void Server::start(int port) {
    this->port = port;
    std::cout << "Server started on port: " << port << std::endl;
    acceptConnections();
}

void Server::acceptConnections() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        return;
    }

    std::cout << "Server is listening for clients..." << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket >= 0) {
            char clientIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);
            std::cout << "Client connected: " << clientIp << std::endl;

            std::thread([=]() {
                this->handleClient(clientSocket);
                }).detach();
		}
		else {
			std::cerr << "Failed to accept client connection.\n";
        }
    }

    closesocket(serverSocket);
}

void Server::handleClient(int clientSocket) {
    char filenameBuffer[256] = { 0 };

    int bytesRead = recv(clientSocket, filenameBuffer, sizeof(filenameBuffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to receive telemetry filename.\n";
        closesocket(clientSocket);
        return;
    }

    std::string telemetryFile(filenameBuffer);
    std::cout << "Client sent telemetry file to use: " << telemetryFile << std::endl;

    closesocket(clientSocket);

    DataParser parser(telemetryFile);
    std::vector<double> fuelValues = parser.parseFuelData();

    Flight flight(1000 + rand() % 1000);
    flight.setStartTime();

    for (double fuel : fuelValues) {
        flight.addFuelData(fuel);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    flight.setEndTime();
    flight.calculateFuelConsumption();

    storeFlightData(flight);
}

void Server::storeFlightData(const Flight& data) {
    std::lock_guard<std::mutex> lock(flightDataMutex);
    flightData.push_back(data);
    flightData.back().saveData("Flight_ID.txt");
    std::cout << "Flight stored: " << flightData.back().getFlightDetails() << std::endl;
}