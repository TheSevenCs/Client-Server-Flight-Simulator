#define _CRT_SECURE_NO_WARNINGS
#include "Flight.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// Constructor initializes flight ID and sets default fuel consumption to zero
Flight::Flight(int id)
    : flightID(id), fuelConsumed(0.0), averageFuelConsumption(0.0) {
}

// Setting the flight start time
void Flight::setStartTime(const std::chrono::system_clock::time_point& t) {
    startTime = t;
}

void Flight::setEndTime(const std::chrono::system_clock::time_point& t) {
    endTime = t;
}

// Add fuel data points
void Flight::addFuelData(double fuel) {
    fuelData.push_back(fuel);
}

// Calculation of total fuel consumption and average fuel consumption rate
void Flight::calculateFuelConsumption() {
    if (fuelData.size() < 2) {
        fuelConsumed = 0.0;
        averageFuelConsumption = 0.0;
        return;
    }

    // Calculate total fuel consumption (first reading - last reading)
    fuelConsumed = fuelData.front() - fuelData.back();

    // Calculation of flight duration in hours
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    double durationHours = duration / 3600.0;

    // Calculate average fuel consumption rate (gallons per hour)
    if (durationHours > 0) {
        averageFuelConsumption = fuelConsumed / durationHours;
    }
    else {
        averageFuelConsumption = 0.0;
    }
}

// Save flight data to file
void Flight::saveData(const std::string& filename) const {
    std::ofstream file(filename);
    if (file) {
        // Convert points in time to a readable format
        auto startTimeT = std::chrono::system_clock::to_time_t(startTime);
        auto endTimeT = std::chrono::system_clock::to_time_t(endTime);

        file << "Flight ID: " << flightID << "\n";
        file << "Start Time: " << std::put_time(std::localtime(&startTimeT), "%Y-%m-%d %H:%M:%S") << "\n";
        file << "End Time: " << std::put_time(std::localtime(&endTimeT), "%Y-%m-%d %H:%M:%S") << "\n";
        file << "Fuel Consumed: " << fuelConsumed << " gallons\n";
        file << "Average Fuel Consumption: " << averageFuelConsumption << " gallons/hour\n";
        file << "Number of Data Points: " << fuelData.size() << "\n";

        // Optional: add detailed fuel data
        file << "\nDetailed Fuel Data:\n";
        for (size_t i = 0; i < fuelData.size(); ++i) {
            file << "Data Point " << i + 1 << ": " << fuelData[i] << " gallons\n";
        }
    }
    else {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
    }
}

// Get the flight ID
int Flight::getFlightID() const {
    return flightID;
}

// Get fuel consumption
double Flight::getFuelConsumed() const {
    return fuelConsumed;
}

// Obtaining the average fuel consumption rate
double Flight::getAverageFuelConsumption() const {
    return averageFuelConsumption;
}

// Get flight details string
std::string Flight::getFlightDetails() const {
    std::stringstream ss;
    ss << "Flight ID: " << flightID
        << ", Fuel Consumed: " << fuelConsumed << " gallons"
        << ", Average Consumption: " << averageFuelConsumption << " gallons/hour";
    return ss.str();
}