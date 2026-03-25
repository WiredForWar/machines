#include "network/RendezvousWorker.hpp"

#include "rendezvous/RendezvousClient.hpp"

#include "spdlog/spdlog.h"

#include <chrono>
#include <utility>

static constexpr std::chrono::milliseconds DefaultStunTimeout{1500};

RendezvousWorker::RendezvousWorker(Rendezvous::RendezvousClientConfig rendezvousConfig,
                                   std::string stunHost,
                                   uint16_t stunPort)
    : client_(std::make_unique<Rendezvous::Client>(std::move(rendezvousConfig)))
    , stunHost_(std::move(stunHost))
    , stunPort_(stunPort)
    , thread_([this](std::stop_token stopToken) { run(std::move(stopToken)); })
{
}

RendezvousWorker::~RendezvousWorker()
{
    // std::jthread requests stop and joins automatically.
}

// ---------------------------------------------------------------------------
// Post requests
// ---------------------------------------------------------------------------

void RendezvousWorker::requestStunQuery(int socketFd)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        stunInFlight_ = true;
    }

    std::string host = stunHost_;
    uint16_t port = stunPort_;

    enqueue([this, host = std::move(host), port, socketFd] {
        StunClient stun(host, port);
        std::optional<StunClient::Result> result = stun.queryFromSocket(socketFd, DefaultStunTimeout);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            stunResult_ = std::move(result);
            stunInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestRegisterSession(Rendezvous::RegisterSessionRequest request)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        registerSessionInFlight_ = true;
    }

    enqueue([this, request = std::move(request)] {
        Rendezvous::RegisterSessionResponse response{};
        bool ok = client_->registerSession(request, &response);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            if (ok)
            {
                registerSessionResult_ = std::move(response);
            }
            else
            {
                registerSessionResult_ = std::nullopt;
            }
            registerSessionInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestHeartbeat(const std::string& sessionId)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        heartbeatInFlight_ = true;
    }

    std::string id = sessionId;
    enqueue([this, id = std::move(id)] {
        bool ok = client_->sendHeartbeat(id);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            heartbeatResult_ = ok;
            heartbeatInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestListSessions()
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        listSessionsInFlight_ = true;
    }

    enqueue([this] {
        std::optional<std::vector<Rendezvous::Session>> sessions = client_->listSessions();

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            listSessionsResult_ = std::move(sessions);
            listSessionsInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestRegisterPunch(const std::string& sessionId, Rendezvous::RegisterPunchRequest request)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        registerPunchInFlight_ = true;
    }

    std::string id = sessionId;
    enqueue([this, id = std::move(id), request = std::move(request)] {
        Rendezvous::RegisterPunchResponse response{};
        bool ok = client_->registerPunchRequest(id, request, &response);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            if (ok)
            {
                registerPunchResult_ = std::move(response);
            }
            else
            {
                registerPunchResult_ = std::nullopt;
            }
            registerPunchInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestRegisterRelay(const std::string& sessionId)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        registerRelayInFlight_ = true;
    }

    std::string id = sessionId;
    enqueue([this, id = std::move(id)] {
        Rendezvous::RegisterRelayResponse response{};
        bool ok = client_->registerRelayRequest(id, &response);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            if (ok)
            {
                registerRelayResult_ = std::move(response);
            }
            else
            {
                registerRelayResult_ = std::nullopt;
            }
            registerRelayInFlight_ = false;
        }
    });
}

void RendezvousWorker::requestListRequests(const std::string& sessionId)
{
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        listRequestsInFlight_ = true;
    }

    std::string id = sessionId;
    enqueue([this, id = std::move(id)] {
        std::optional<std::vector<Rendezvous::ConnectionRequestInfo>> result = client_->listRequests(id);

        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            listRequestsResult_ = std::move(result);
            listRequestsInFlight_ = false;
        }
    });
}

// ---------------------------------------------------------------------------
// Consume results
// ---------------------------------------------------------------------------

std::optional<std::optional<StunClient::Result>> RendezvousWorker::takeStunResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<StunClient::Result>> out{};
    out.swap(stunResult_);
    return out;
}

std::optional<std::optional<Rendezvous::RegisterSessionResponse>> RendezvousWorker::takeRegisterSessionResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<Rendezvous::RegisterSessionResponse>> out{};
    out.swap(registerSessionResult_);
    return out;
}

std::optional<bool> RendezvousWorker::takeHeartbeatResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<bool> out{};
    out.swap(heartbeatResult_);
    return out;
}

std::optional<std::optional<std::vector<Rendezvous::Session>>> RendezvousWorker::takeListSessionsResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<std::vector<Rendezvous::Session>>> out{};
    out.swap(listSessionsResult_);
    return out;
}

std::optional<std::optional<Rendezvous::RegisterPunchResponse>> RendezvousWorker::takeRegisterPunchResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<Rendezvous::RegisterPunchResponse>> out{};
    out.swap(registerPunchResult_);
    return out;
}

std::optional<std::optional<Rendezvous::RegisterRelayResponse>> RendezvousWorker::takeRegisterRelayResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<Rendezvous::RegisterRelayResponse>> out{};
    out.swap(registerRelayResult_);
    return out;
}

std::optional<std::optional<std::vector<Rendezvous::ConnectionRequestInfo>>> RendezvousWorker::takeListRequestsResult()
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    std::optional<std::optional<std::vector<Rendezvous::ConnectionRequestInfo>>> out{};
    out.swap(listRequestsResult_);
    return out;
}

// ---------------------------------------------------------------------------
// In-flight queries
// ---------------------------------------------------------------------------

bool RendezvousWorker::isStunInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return stunInFlight_;
}

bool RendezvousWorker::isRegisterSessionInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return registerSessionInFlight_;
}

bool RendezvousWorker::isHeartbeatInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return heartbeatInFlight_;
}

bool RendezvousWorker::isListSessionsInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return listSessionsInFlight_;
}

bool RendezvousWorker::isRegisterPunchInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return registerPunchInFlight_;
}

bool RendezvousWorker::isRegisterRelayInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return registerRelayInFlight_;
}

bool RendezvousWorker::isListRequestsInFlight() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return listRequestsInFlight_;
}

// ---------------------------------------------------------------------------
// Thread internals
// ---------------------------------------------------------------------------

void RendezvousWorker::enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push(std::move(task));
    }
    cv_.notify_all();
}

void RendezvousWorker::run(std::stop_token stopToken)
{
    spdlog::debug("RendezvousWorker: thread started");

    while (!stopToken.stop_requested())
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait(lock, stopToken, [this] { return !queue_.empty(); });

            if (stopToken.stop_requested() && queue_.empty())
            {
                break;
            }

            if (queue_.empty())
            {
                continue;
            }

            task = std::move(queue_.front());
            queue_.pop();
        }

        task();
    }

    spdlog::debug("RendezvousWorker: thread stopped");
}
