#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <map>
#include <mutex>
#include <string>
#include "Flight.h"
#include "UniqueIDManager.h"

class Server {
public:
    Server();
    ~Server();

    void start(int port);
    void stop();

private:
    int port;
    int serverSocket;
    bool running;
    std::map<int, Flight> activeFlights;  // Active flights, indexed by aircraft ID
    std::mutex flightsMutex;
    UniqueIDManager idManager;
    void acceptConnections();
    void handleClient(int clientSocket);
    void storeFlightData(const Flight& flight);

    // Parsing telemetry data from network packets
    std::pair<int, std::pair<std::string, double>> parseTelemPacket(const std::vector<char>& packet);

    // Get or create a Flight object associated with a specific aircraft ID
    Flight& getOrCreateFlight(int aircraftId);
};

#endif