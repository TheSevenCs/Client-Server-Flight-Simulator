#include "Server.h"
#include <iostream>

void Server::start(int port) {
    std::cout << "Server started on port: " << port << std::endl;
    acceptConnections();
}

void Server::acceptConnections() {
    std::cout << "Accepting client connections..." << std::endl;

    // Create a simulated client
    Client client("127.0.0.1", port);
    Aircraft aircraft(1001, "SimulatedJet");
    aircraft.setInitialFuel(5000.0);
    client.setAircraft(aircraft);

    clients.push_back(client);
}

void Server::receiveData() {
    for (Client& client : clients) {
        // Simulated telemetry file path (change if needed)
        std::string telemetryFile = "telemetry.txt";

        // Parse file using DataParser
        DataParser parser(telemetryFile);
        std::vector<double> fuelValues = parser.parseFuelData();

        Flight flight(client.getAircraft().getID());
        flight.setStartTime();

        for (double fuel : fuelValues) {
            flight.addFuelData(fuel);

            // Simulate telemetry transmission
            TelemetryData data(std::chrono::system_clock::now(), fuel, client.getAircraft().getID());
            data.packetize();
            client.transmitTelemetryData(data);
        }

        flight.setEndTime();
        flight.calculateFuelConsumption();
        storeFlightData(flight);
    }
}

void Server::storeFlightData(const Flight& data) {
    flightData.push_back(data);
    flightData.back().saveData("Flight_ID.txt");
    std::cout << "Flight stored: " << flightData.back().getFlightDetails() << std::endl;
}
