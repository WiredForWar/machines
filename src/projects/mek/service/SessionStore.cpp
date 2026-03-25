#include "service/SessionStore.hpp"

#include <sstream>
#include <utility>

SessionStore::SessionStore(std::chrono::seconds sessionTimeout)
    : sessionTimeout_(sessionTimeout)
{
}

SessionStore::Session SessionStore::registerSession(
    const std::string& hostAddress,
    uint16_t hostPort,
    const std::string& hostLanAddress,
    uint16_t hostLanPort,
    const std::string& gameName)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> guard(mutex_);

    pruneExpiredLocked(now);

    for (auto iterator = sessions_.begin(); iterator != sessions_.end();)
    {
        const Session& existing = iterator->second;
        if (existing.hostAddress == hostAddress && existing.hostPort == hostPort)
        {
            iterator = sessions_.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }

    Session session{};
    session.sessionId = generateSessionId();
    session.hostAddress = hostAddress;
    session.hostPort = hostPort;
    session.hostLanAddress = hostLanAddress;
    session.hostLanPort = hostLanPort;
    session.gameName = gameName;
    session.createdAt = now;
    session.lastHeartbeat = now;

    sessions_[session.sessionId] = session;
    return session;
}

std::vector<SessionStore::Session> SessionStore::sessions() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<SessionStore::Session> copy{};
    copy.reserve(sessions_.size());

    for (const std::pair<const std::string, Session>& entry : sessions_)
    {
        copy.push_back(entry.second);
    }

    return copy;
}

bool SessionStore::heartbeat(const std::string& sessionId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const auto iterator = sessions_.find(sessionId);
    if (iterator == sessions_.end())
    {
        return false;
    }

    iterator->second.lastHeartbeat = now;
    return true;
}

std::optional<Rendezvous::RegisterPunchResponse> SessionStore::registerPunchRequest(
    const std::string& sessionId,
    const std::string& clientAddress,
    uint16_t clientPort)
{
    if (clientPort == 0)
    {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> guard(mutex_);
    const auto sessionIterator = sessions_.find(sessionId);
    if (sessionIterator == sessions_.end())
    {
        return std::nullopt;
    }

    Rendezvous::PunchRequestInfo info{};
    {
        std::ostringstream stream;
        stream << std::hex << std::uppercase << nextRequestId_++;
        info.requestId = stream.str();
    }
    info.clientAddress = clientAddress;
    info.clientPort = clientPort;

    punchRequests_[sessionId].push_back(info);

    Rendezvous::RegisterPunchResponse response{};
    response.requestId = info.requestId;
    return response;
}

std::optional<std::vector<Rendezvous::PunchRequestInfo>> SessionStore::consumePunchRequests(const std::string& sessionId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    const auto sessionIterator = sessions_.find(sessionId);
    if (sessionIterator == sessions_.end())
    {
        return std::nullopt;
    }

    auto queueIterator = punchRequests_.find(sessionId);
    if (queueIterator == punchRequests_.end())
    {
        return std::vector<Rendezvous::PunchRequestInfo>{};
    }

    std::vector<Rendezvous::PunchRequestInfo> pending = std::move(queueIterator->second);
    punchRequests_.erase(queueIterator);
    return pending;
}

std::optional<Rendezvous::RegisterRelayResponse> SessionStore::registerRelayRequest(
    const std::string& sessionId,
    const std::string& relayAddress,
    uint16_t relayHostPort,
    uint16_t relayClientPort)
{
    std::lock_guard<std::mutex> guard(mutex_);
    const auto sessionIterator = sessions_.find(sessionId);
    if (sessionIterator == sessions_.end())
    {
        return std::nullopt;
    }

    Rendezvous::RelayRequestInfo info{};
    {
        std::ostringstream stream;
        stream << std::hex << std::uppercase << nextRequestId_++;
        info.requestId = stream.str();
    }
    info.relayAddress = relayAddress;
    info.relayHostPort = relayHostPort;
    info.relayClientPort = relayClientPort;

    relayRequests_[sessionId].push_back(info);

    Rendezvous::RegisterRelayResponse response{};
    response.requestId = info.requestId;
    response.relayAddress = relayAddress;
    response.relayHostPort = relayHostPort;
    response.relayClientPort = relayClientPort;
    return response;
}

std::optional<std::vector<Rendezvous::ConnectionRequestInfo>> SessionStore::consumeConnectionRequests(
    const std::string& sessionId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    const auto sessionIterator = sessions_.find(sessionId);
    if (sessionIterator == sessions_.end())
    {
        return std::nullopt;
    }

    std::vector<Rendezvous::ConnectionRequestInfo> result;

    auto punchIterator = punchRequests_.find(sessionId);
    if (punchIterator != punchRequests_.end())
    {
        for (const Rendezvous::PunchRequestInfo& punch : punchIterator->second)
        {
            Rendezvous::ConnectionRequestInfo info{};
            info.type = "punch";
            info.requestId = punch.requestId;
            info.clientAddress = punch.clientAddress;
            info.clientPort = punch.clientPort;
            result.push_back(std::move(info));
        }
        punchRequests_.erase(punchIterator);
    }

    auto relayIterator = relayRequests_.find(sessionId);
    if (relayIterator != relayRequests_.end())
    {
        for (const Rendezvous::RelayRequestInfo& relay : relayIterator->second)
        {
            Rendezvous::ConnectionRequestInfo info{};
            info.type = "relay";
            info.requestId = relay.requestId;
            info.relayAddress = relay.relayAddress;
            info.relayHostPort = relay.relayHostPort;
            info.relayClientPort = relay.relayClientPort;
            result.push_back(std::move(info));
        }
        relayRequests_.erase(relayIterator);
    }

    return result;
}

void SessionStore::pruneExpired()
{
    std::lock_guard<std::mutex> guard(mutex_);
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    pruneExpiredLocked(now);
}

std::string SessionStore::generateSessionId()
{
    std::ostringstream stream{};
    stream << std::hex << std::uppercase << nextSessionId_++;
    return stream.str();
}

void SessionStore::pruneExpiredLocked(std::chrono::steady_clock::time_point now)
{
    for (auto iterator = sessions_.begin(); iterator != sessions_.end();)
    {
        const std::chrono::steady_clock::duration elapsed = now - iterator->second.lastHeartbeat;
        if (elapsed > sessionTimeout_)
        {
            iterator = sessions_.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}
