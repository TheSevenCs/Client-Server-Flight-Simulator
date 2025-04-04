#pragma once
#include <atomic>
#include <mutex>

class UniqueIDManager {
private:
    static UniqueIDManager* instance;
    std::atomic<int> nextID;
    std::mutex idMutex;

    UniqueIDManager();  // Private constructor for singleton

public:
    static UniqueIDManager* getInstance();
    int generateUniqueID();
    bool isIDValid(int id);
    void releaseID(int id);
};