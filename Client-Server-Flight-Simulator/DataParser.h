#pragma once
#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <string>
#include <vector>

class DataParser {
private:
    std::vector<std::string> rawData;

public:
    DataParser(const std::string& filename);

    void extractData();
    std::vector<double> parseFuelData() const;
    std::vector<std::string> parseTimeData() const;
};

#endif

