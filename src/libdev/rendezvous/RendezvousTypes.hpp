#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace Rendezvous
{

struct Session
{
    std::string sessionId{};
    std::string hostAddress{};
    uint16_t hostPort{};
    std::string gameName{};
    std::chrono::steady_clock::time_point createdAt{};
    std::chrono::steady_clock::time_point lastHeartbeat{};
};

struct RegisterSessionRequest
{
    uint16_t hostPort{};
    std::string gameName{};
};

struct RegisterSessionResponse
{
    Session session{};
    std::chrono::seconds expiresIn{};
};

struct HeartbeatRequest
{
};

struct RegisterPunchRequest
{
    uint16_t clientPort{};
};

struct RegisterPunchResponse
{
    std::string requestId{};
};

struct PunchRequestInfo
{
    std::string requestId{};
    std::string clientAddress{};
    uint16_t clientPort{};
};

struct RendezvousClientConfig
{
    std::string host{};
    uint16_t port{};
    bool useHttps{};
    std::string apiPathPrefix{};
    std::chrono::milliseconds networkTimeout{};
};

} // namespace Rendezvous
