#include "UniqueIDManager.h"

UniqueIDManager::UniqueIDManager() : currentID(1) {}

int UniqueIDManager::generateID() {
    std::lock_guard<std::mutex> lock(idMutex);
    return currentID++;
}
