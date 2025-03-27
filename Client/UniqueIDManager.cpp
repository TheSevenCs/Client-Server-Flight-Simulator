#include "UniqueIDManager.h"
#include <set>

UniqueIDManager* UniqueIDManager::instance = nullptr;

UniqueIDManager::UniqueIDManager() : nextID(1000) {
    // Start with ID 1000
}

UniqueIDManager* UniqueIDManager::getInstance() {
    if (instance == nullptr) {
        instance = new UniqueIDManager();
    }
    return instance;
}

int UniqueIDManager::generateUniqueID() {
    std::lock_guard<std::mutex> lock(idMutex);
    return nextID++;
}

bool UniqueIDManager::isIDValid(int id) {
    return id >= 1000;
}

void UniqueIDManager::releaseID(int id) {
    // Could implement a system to reuse IDs if needed
}