#include "Client.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string serverIP = "127.0.0.1";  // Default IP
    int serverPort = 8080;              // Default port
    std::string telemetryFile = "";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--ip" && i + 1 < argc) {
            serverIP = argv[i + 1];
            i++;
        }
        else if (arg == "--port" && i + 1 < argc) {
            serverPort = std::stoi(argv[i + 1]);
            i++;
        }
        else if (arg == "--file" && i + 1 < argc) {
            telemetryFile = argv[i + 1];
            i++;
        }
        else if (telemetryFile.empty()) {
            // Assume it's the telemetry file if no other parameter is specified
            telemetryFile = arg;
        }
    }

    // Check if telemetry file is provided
    if (telemetryFile.empty()) {
        std::cout << "Usage: Client.exe [--ip <server_ip>] [--port <server_port>] --file <telemetry_file>" << std::endl;
        return 1;
    }

    // Create client
    Client client(serverIP, serverPort);

    // Initialize with telemetry file
    if (!client.initialize(telemetryFile)) {
        std::cerr << "Failed to initialize client with telemetry file." << std::endl;
        return 1;
    }

    // Connect to server
    std::cout << "Connecting to server at " << serverIP << ":" << serverPort << "..." << std::endl;
    if (!client.connectToServer()) {
        std::cerr << "Failed to connect to server." << std::endl;
        return 1;
    }

    std::cout << "Connected to server. Aircraft ID: " << client.getAircraft().getID() << std::endl;
    std::cout << "Processing telemetry file: " << telemetryFile << std::endl;

    // Process telemetry file
    if (!client.processFile()) {
        std::cerr << "Failed to process telemetry file." << std::endl;
        client.disconnectFromServer();
        return 1;
    }

    // Disconnect from server
    client.disconnectFromServer();
    std::cout << "Client disconnected. Exiting." << std::endl;

    return 0;
}