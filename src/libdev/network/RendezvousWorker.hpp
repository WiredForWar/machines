#pragma once

#include "network/StunClient.hpp"
#include "rendezvous/RendezvousTypes.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace Rendezvous
{
class Client;
} // namespace Rendezvous

// Runs all blocking network I/O (HTTP and STUN) on a dedicated worker
// thread so the game's main thread never stalls.
//
// The main thread posts requests via requestXxx() methods and polls for
// results via takeXxxResult() methods every frame. Results are stored in
// typed optional slots protected by a mutex; the lock is held only for
// the duration of a move/swap so the main thread is never meaningfully
// blocked.
class RendezvousWorker
{
public:
    explicit RendezvousWorker(Rendezvous::RendezvousClientConfig rendezvousConfig,
                              std::string stunHost,
                              uint16_t stunPort);
    ~RendezvousWorker();

    RendezvousWorker(const RendezvousWorker&) = delete;
    RendezvousWorker& operator=(const RendezvousWorker&) = delete;

    // --- Post requests (called from main thread, non-blocking) ----------

    // Discover public endpoint via STUN. When `socketFd` >= 0 the query
    // is sent from that socket (so the mapped port matches ENet's port).
    void requestStunQuery(int socketFd);

    void requestRegisterSession(Rendezvous::RegisterSessionRequest request);
    void requestHeartbeat(const std::string& sessionId);
    void requestListSessions();
    void requestRegisterPunch(const std::string& sessionId, Rendezvous::RegisterPunchRequest request);
    void requestListRequests(const std::string& sessionId);

    // --- Consume results (called from main thread, non-blocking) --------
    // Each returns std::nullopt if no result is ready yet.

    std::optional<std::optional<StunClient::Result>> takeStunResult();
    std::optional<std::optional<Rendezvous::RegisterSessionResponse>> takeRegisterSessionResult();
    std::optional<bool> takeHeartbeatResult();
    std::optional<std::optional<std::vector<Rendezvous::Session>>> takeListSessionsResult();
    std::optional<std::optional<Rendezvous::RegisterPunchResponse>> takeRegisterPunchResult();
    std::optional<std::optional<std::vector<Rendezvous::PunchRequestInfo>>> takeListRequestsResult();

    // --- Status queries (called from main thread) -----------------------

    bool isStunInFlight() const;
    bool isRegisterSessionInFlight() const;
    bool isHeartbeatInFlight() const;
    bool isListSessionsInFlight() const;
    bool isRegisterPunchInFlight() const;
    bool isListRequestsInFlight() const;

private:
    using Task = std::function<void()>;

    void enqueue(Task task);
    void run(std::stop_token stopToken);

    // Worker thread's own client — only touched from the worker thread.
    std::unique_ptr<Rendezvous::Client> client_{};
    std::string stunHost_{};
    uint16_t stunPort_{};

    // Thread machinery
    std::mutex queueMutex_{};
    std::condition_variable_any cv_{};
    std::queue<Task> queue_{};
    std::jthread thread_{};

    // Result slots — written by worker thread, read by main thread.
    mutable std::mutex resultMutex_{};

    std::optional<std::optional<StunClient::Result>> stunResult_{};
    bool stunInFlight_{};

    std::optional<std::optional<Rendezvous::RegisterSessionResponse>> registerSessionResult_{};
    bool registerSessionInFlight_{};

    std::optional<bool> heartbeatResult_{};
    bool heartbeatInFlight_{};

    std::optional<std::optional<std::vector<Rendezvous::Session>>> listSessionsResult_{};
    bool listSessionsInFlight_{};

    std::optional<std::optional<Rendezvous::RegisterPunchResponse>> registerPunchResult_{};
    bool registerPunchInFlight_{};

    std::optional<std::optional<std::vector<Rendezvous::PunchRequestInfo>>> listRequestsResult_{};
    bool listRequestsInFlight_{};
};
