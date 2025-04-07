#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "SocketDataTransmitter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <ctime>

Client::Client(const std::string& serverIP, int serverPort)
    : serverIP(serverIP), serverPort(serverPort), isConnected(false) {

  
    transmitter = std::make_unique<SocketDataTransmitter>();

    aircraft = Aircraft(0);
}

Client::~Client() {
    disconnectFromServer();
}

bool Client::initialize(const std::string& telemetryFilePath) {
    this->telemetryFilePath = telemetryFilePath;

    // Verify that the file exists
    std::ifstream file(telemetryFilePath);
    if (!file) {
        std::cerr << "Failed to open telemetry file: " << telemetryFilePath << std::endl;
        return false;
    }

  
    std::string line;
    if (std::getline(file, line)) {
        // file size
        size_t firstComma = line.find(',');
        if (firstComma != std::string::npos) {
            size_t secondComma = line.find(',', firstComma + 1);
            if (secondComma != std::string::npos) {
                std::string fuelStr = line.substr(secondComma + 1);
                fuelStr.erase(std::remove_if(fuelStr.begin(), fuelStr.end(),
                    [](char c) { return c == ' ' || c == ',' || c == '\r' || c == '\n'; }), fuelStr.end());
                try {
                    double initialFuel = std::stod(fuelStr);
                    aircraft.setInitialFuel(initialFuel);
                }
                catch (...) {
                    std::cerr << "Failed to parse initial fuel value." << std::endl;
                }
            }
        }
    }

    file.close();
    return true;
}

//bool Client::connectToServer() {
//    if (isConnected) {
//        return true;
//    }
//
//    isConnected = transmitter->connect(serverIP, serverPort);
//
//    if (isConnected) {
//        // Creating the initial packet
//        auto now = std::chrono::system_clock::now();
//        TelemetryData initialData(now, aircraft.getInitialFuel(), aircraft.getID());
//        initialData.packetize();
//
//        // Send initial pack
//        if (!transmitter->send(initialData.getPacketizedData())) {
//            isConnected = false;
//            return false;
//        }
//    }
//
//    return isConnected;
//}

bool Client::connectToServer() {
    if (isConnected) {
        return true;
    }

    isConnected = transmitter->connect(serverIP, serverPort);

    if (isConnected) {
        // 接收服务器分配的ID
        std::vector<char> buffer(64, 0); // 创建足够大的缓冲区
        bool idReceived = transmitter->receive(buffer);

        if (!idReceived || buffer.empty()) {
            std::cerr << "Failed to receive ID from server." << std::endl;
            disconnectFromServer();
            return false;
        }

        // 解析ID (假设服务器只发送数字)
        std::string idStr(buffer.begin(), buffer.end());
        idStr = idStr.substr(0, idStr.find('\0')); // 去除尾部空字符

        try {
            int id = std::stoi(idStr);
            // 设置飞机ID
            aircraft.setID(id);
            std::cout << "Received ID from server: " << id << std::endl;
        }
        catch (...) {
            std::cerr << "Failed to parse ID: " << idStr << std::endl;
            disconnectFromServer();
            return false;
        }

        // 现在我们有了ID，创建初始数据包


    }

    return isConnected;
}


void Client::disconnectFromServer() {
    if (transmitter) {
        transmitter->disconnect();
    }
    isConnected = false;
}

bool Client::processFile() {
    if (!isConnected) {
        std::cerr << "Not connected to server." << std::endl;
        return false;
    }

    std::ifstream file(telemetryFilePath);
    if (!file) {
        std::cerr << "Failed to open telemetry file: " << telemetryFilePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 去除行首尾的空白字符
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        // 格式: 1_3_2023 12:35:34,33.571247,
        size_t comma = line.find(',');
        if (comma == std::string::npos) continue;

        std::string timeStr = line.substr(0, comma);
        std::string fuelStr = line.substr(comma + 1);

        // 清理时间和燃油字符串
        timeStr.erase(0, timeStr.find_first_not_of(" \t\r\n"));
        timeStr.erase(timeStr.find_last_not_of(" \t\r\n") + 1);

        fuelStr.erase(0, fuelStr.find_first_not_of(" \t\r\n"));
        // 移除尾部的逗号和空白
        size_t lastNonComma = fuelStr.find_last_not_of(",\t\r\n ");
        if (lastNonComma != std::string::npos) {
            fuelStr = fuelStr.substr(0, lastNonComma + 1);
        }

        double fuelRemaining = 0.0;
        try {
            fuelRemaining = std::stod(fuelStr);
        }
        catch (...) {
            std::cerr << "Failed to parse fuel value: '" << fuelStr << "'" << std::endl;
            continue; // 跳过解析失败的行
        }

        // 创建时间点 - 解析 "1_3_2023 12:35:34" 格式的时间
        std::tm tm = {};
        int month, day, year, hour, min, sec;
        if (sscanf(timeStr.c_str(), "%d_%d_%d %d:%d:%d",
            &month, &day, &year, &hour, &min, &sec) == 6) {
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = min;
            tm.tm_sec = sec;

            auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            // 创建遥测数据
            TelemetryData data(time_point, fuelRemaining, aircraft.getID());

            // 传输到服务器
            if (!transmitTelemetryData(data)) {
                std::cerr << "Failed to transmit telemetry data." << std::endl;
                file.close();
                return false;
            }

            // 延迟50毫秒，模拟实时数据传输
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else {
            std::cerr << "Failed to parse time: '" << timeStr << "'" << std::endl;
        }
    }

    file.close();
    std::cout << "File processing completed." << std::endl;
    return true;
}

bool Client::transmitTelemetryData(const TelemetryData& data) {
    if (!isConnected) {
        return false;
    }

    // Serialized data
    TelemetryData packetData = data;
    packetData.packetize();

    // Send data to the server
    return transmitter->send(packetData.getPacketizedData());
}

const Aircraft& Client::getAircraft() const {
    return aircraft;
}

void Client::setAircraft(const Aircraft& aircraft) {
    this->aircraft = aircraft;
}

bool Client::isServerConnected() const {
    return isConnected && transmitter->isConnected();
}
