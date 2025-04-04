#include "Server.h"
#include <iostream>

void Server::start(int port) {
    std::cout << "Server started on port: " << port << std::endl;
    acceptConnections();
}

void Server::acceptConnections() {
    std::cout << "Accepting client connections..." << std::endl;
    Client client;
    client.setUniqueID("CL001");
    clients.push_back(client);
}

void Server::receiveData() {
    for (Client& client : clients) {
        std::vector<TelemetryData> dataList = client.readTelemetryFile();

        for (const auto& data : dataList) {
            Packet packet = client.packetizeData(data);
            client.transmitData(packet);

            Flight flight;
            flight.calculateFuelConsumption();
            storeFlightData(flight);
        }
    }
}

void Server::storeFlightData(const Flight& data) {
    flightData.push_back(data);
    flightData.back().saveData();
    std::cout << "Flight stored: " << flightData.back().getFlightDetails() << std::endl;
}
