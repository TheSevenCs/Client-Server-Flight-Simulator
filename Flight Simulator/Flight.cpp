#include "Flight.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// Constructor that will initialize flight ID and sets default fuel consumption to zero
Flight::Flight(int id) : flightID(id), fuelConsumed(0.0) {}

// This function sets the start time of the flight using the current system time
void Flight::setStartTime() {
    startTime = std::chrono::system_clock::now();
}

// This function sets the end time of the flight using the current system time
void Flight::setEndTime() {
    endTime = std::chrono::system_clock::now();
}

// This function adds a fuel data point to the fuelData vector for tracking consumption
void Flight::addFuelData(double fuel) {
    fuelData.push_back(fuel);
}

// This function calculates the total fuel consumption based on the recorded fuel data
void Flight::calculateFuelConsumption() {
    if (!fuelData.empty()) {
        // Assumes the first entry is initial fuel and the last entry is final fuel level
        fuelConsumed = fuelData.front() - fuelData.back();
    }
}

// This function saves flight details and fuel consumption to a file
void Flight::saveData(const std::string& filename) const {
    std::ofstream file(filename);
    if (file) {
        file << "Flight ID: " << flightID << "\n";
        file << "Fuel Consumed: " << fuelConsumed << " liters\n";
    }
}

// This function returns a formatted string with flight details for display
std::string Flight::getFlightDetails() const {
    std::stringstream ss;
    ss << "Flight ID: " << flightID << ", Fuel Consumed: " << fuelConsumed << " liters";
    return ss.str();
}
