#pragma once
#include <vector>
#include "../Client/Client.h"
#include "Flight.h"
#include "Server.h"
#include "../Client/TelemetryData.h"
#include "DataParser.h"


class Server {
public:
    void start(int port);
    void acceptConnections();
    void handleClient(int clientSocket);
    void receiveData();
    void storeFlightData(const Flight&);

private:
    int port;
    std::vector<Client> clients;
    std::vector<Flight> flightData;

    void handleClient(Client& client);
};