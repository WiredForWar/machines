#pragma once

#include <cstdint>
#include <string>

class NetSessionInfo
{
public:
    std::string address; // ip:port (public)
    std::string lanAddress; // ip:port (LAN, for same-NAT detection)
    std::string serverName;
    uint32_t gameVersion{};
    std::string sessionId;
};
