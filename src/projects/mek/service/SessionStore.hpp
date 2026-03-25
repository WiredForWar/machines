#pragma once

#include "rendezvous/RendezvousTypes.hpp"

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class SessionStore
{
public:
    using Session = Rendezvous::Session;

    explicit SessionStore(std::chrono::seconds sessionTimeout);

    Session registerSession(
        const std::string& hostAddress,
        uint16_t hostPort,
        const std::string& hostLanAddress,
        uint16_t hostLanPort,
        const std::string& gameName);

    std::vector<Session> sessions() const;

    bool heartbeat(const std::string& sessionId);

    std::optional<Rendezvous::RegisterPunchResponse> registerPunchRequest(
        const std::string& sessionId,
        const std::string& clientAddress,
        uint16_t clientPort);

    std::optional<std::vector<Rendezvous::PunchRequestInfo>> consumePunchRequests(const std::string& sessionId);

    void pruneExpired();

private:
    std::string generateSessionId();
    void pruneExpiredLocked(std::chrono::steady_clock::time_point now);

    std::chrono::seconds sessionTimeout_;
    mutable std::mutex mutex_{};
    std::unordered_map<std::string, Session> sessions_{};
    uint64_t nextSessionId_{1};
    uint64_t nextPunchRequestId_{1};
    std::unordered_map<std::string, std::vector<Rendezvous::PunchRequestInfo>> punchRequests_{};
};
