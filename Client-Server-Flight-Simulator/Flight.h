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
    double averageFuelConsumption;

public:
    Flight(int id);

    void setStartTime(const std::chrono::system_clock::time_point& t);
    void setEndTime(const std::chrono::system_clock::time_point& t);


    void addFuelData(double fuel);
    void calculateFuelConsumption();
    void saveData(const std::string& filename) const;

    int getFlightID() const;
    double getFuelConsumed() const;
    double getAverageFuelConsumption() const;
    std::string getFlightDetails() const;
};

#endif