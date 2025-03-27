#include "Aircraft.h"

Aircraft::Aircraft() : id(-1), type("Unknown"), totalFuelUsed(0.0), initialFuel(0.0) {
}

Aircraft::Aircraft(int id, const std::string& type)
    : id(id), type(type), totalFuelUsed(0.0), initialFuel(0.0) {
}

int Aircraft::getID() const {
    return id;
}

void Aircraft::setID(int id) {
    this->id = id;
}

std::string Aircraft::getType() const {
    return type;
}

void Aircraft::setType(const std::string& type) {
    this->type = type;
}

double Aircraft::getTotalFuelUsed() const {
    return totalFuelUsed;
}

void Aircraft::updateFuelUsage(double fuelUsed) {
    totalFuelUsed += fuelUsed;
}

void Aircraft::setInitialFuel(double fuel) {
    initialFuel = fuel;
}

double Aircraft::getInitialFuel() const {
    return initialFuel;
}