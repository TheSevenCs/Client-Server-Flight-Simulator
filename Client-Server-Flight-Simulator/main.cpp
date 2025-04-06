#include "Server.h"
#include <iostream>
#include <csignal>

// Global server pointer for signal processing
Server* serverPtr = nullptr;

// Signal processing functions
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    // Stop the server
    if (serverPtr) {
        serverPtr->stop();
    }

    // Exit the program
    exit(signum);
}

int main(int argc, char* argv[]) {
    // Setting up signal processing
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    int port = 5000; // Default port

    // Parsing command line arguments
    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);
        }
        catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << e.what() << std::endl;
            std::cout << "Usage: " << argv[0] << " [port]" << std::endl;
            return 1;
        }
    }

    try {
        // Create a server instance
        Server server;
        serverPtr = &server;

        // Start the server
        server.start(port);

        // Wait for the user to enter the exit command
        std::cout << "Server running. Press Enter to stop." << std::endl;
        std::cin.get();

        // Stop the server
        server.stop();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}