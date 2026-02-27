#pragma once

#include <cstdint>
#include <string>

class NetSessionInfo
{
public:
    std::string address; // ip:port
    std::string serverName;
    uint32_t gameVersion{};
};
