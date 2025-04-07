#pragma once
#include <mutex>

class UniqueIDManager {
public:
    UniqueIDManager();

    int generateID();

private:
    int currentID;
    std::mutex idMutex;
};
