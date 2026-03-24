#pragma once

#include "rendezvous/RendezvousTypes.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace httplib
{

class Client;
class ClientImpl;

} // namespace httplib

namespace Rendezvous
{

class Client
{
public:
    explicit Client(RendezvousClientConfig config);

    const RendezvousClientConfig& config() const { return config_; }

    bool registerSession(const RegisterSessionRequest& request, RegisterSessionResponse* response) const;
    bool sendHeartbeat(const std::string& sessionId) const;
    std::optional<std::vector<Session>> listSessions() const;
    bool registerPunchRequest(const std::string& sessionId, const RegisterPunchRequest& request, RegisterPunchResponse* response) const;
    std::optional<std::vector<PunchRequestInfo>> listRequests(const std::string& sessionId) const;

private:
    std::unique_ptr<httplib::ClientImpl> createHttpClient() const;
    std::string makePath(const std::string& suffix) const;
    static std::optional<RegisterSessionResponse> parseRegisterResponse(const std::string& body);
    static std::optional<std::vector<Session>> parseSessionList(const std::string& body);
    static std::optional<RegisterPunchResponse> parseRegisterPunchResponse(const std::string& body);
    static std::optional<std::vector<PunchRequestInfo>> parsePunchRequestList(const std::string& body);

    RendezvousClientConfig config_{};
};

} // namespace Rendezvous
