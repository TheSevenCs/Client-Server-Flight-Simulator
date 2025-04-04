#pragma once
#include <vector>
#include "Client.h"
#include "Flight.h"

class Server {
private:
    std::vector<Client> clients;
    std::vector<Flight> flightData;

public:
    void start(int port);
    void acceptConnections();
    void receiveData();
    void storeFlightData(const Flight& data);
};
