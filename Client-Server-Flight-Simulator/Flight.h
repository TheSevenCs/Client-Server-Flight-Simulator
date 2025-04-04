#pragma once
#ifndef FLIGHT_H
#define FLIGHT_H

#include <vector>
#include <string>
#include <chrono>

class Flight {
private:
    int flightID;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    double fuelConsumed;
    std::vector<double> fuelData;

public:
    Flight(int id);

    void setStartTime();
    void setEndTime();

    void addFuelData(double fuel);
    void calculateFuelConsumption();
    void saveData(const std::string& filename) const;

    std::string getFlightDetails() const;
};

#endif

