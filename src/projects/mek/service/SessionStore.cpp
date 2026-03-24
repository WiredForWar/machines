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
        stream << std::hex << std::uppercase << nextPunchRequestId_++;
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
