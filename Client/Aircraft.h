#pragma once
#include <string>

class Aircraft {
private:
    int id;
    std::string type;
    double totalFuelUsed;
    double initialFuel;

public:
    Aircraft();
    Aircraft(int id, const std::string& type = "Unknown");

    int getID() const;
    void setID(int id);
    std::string getType() const;
    void setType(const std::string& type);
    double getTotalFuelUsed() const;
    void updateFuelUsage(double fuelUsed);
    void setInitialFuel(double fuel);
    double getInitialFuel() const;
};