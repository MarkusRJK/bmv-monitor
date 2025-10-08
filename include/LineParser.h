#pragma once

#include <iostream> 
#include <cstdint>
#include <string>

#include "DeviceCache.h"

class LineParser {
public:
    LineParser(DeviceCache& c)
        : mCache(c) {}

    ~LineParser() = default;

    void parse(std::istream& str);
    
private:
    bool isCorrectChecksum(const std::string& line);

    DeviceCache& mCache;
    uint8_t mChecksum = 0;
};

