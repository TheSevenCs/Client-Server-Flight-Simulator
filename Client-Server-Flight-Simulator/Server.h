#pragma once
#include <vector>
#include "../Client/Client.h"
#include "Flight.h"
#include "Server.h"
#include "Flight.h"
#include "../Client/TelemetryData.h"
#include "DataParser.h"


class Server {
public:
    void start(int port);
    void receiveData();

private:
    void acceptConnections();
    void storeFlightData(const Flight& data);

    int port;
    std::vector<Client> clients;
    std::vector<Flight> flightData;
};
