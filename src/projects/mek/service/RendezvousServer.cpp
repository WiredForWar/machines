#include "service/RendezvousServer.hpp"

#include "rendezvous/HttpStatus.hpp"
#include "rendezvous/RendezvousTypes.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <csignal>
#include <sstream>
#include <utility>

namespace
{
constexpr const char* ContentTypeJson = "application/json";
constexpr const char* ContentTypeText = "text/plain";
constexpr std::chrono::seconds MinimumPruneInterval{5};

nlohmann::json makeRegisterSessionResponseJson(const Rendezvous::RegisterSessionResponse& response)
{
    nlohmann::json json;
    json["sessionId"] = response.session.sessionId;
    json["hostAddress"] = response.session.hostAddress;
    json["hostPort"] = response.session.hostPort;
    json["hostLanAddress"] = response.session.hostLanAddress;
    json["hostLanPort"] = response.session.hostLanPort;
    json["gameName"] = response.session.gameName;
    json["expiresInSeconds"] = response.expiresIn.count();
    return json;
}

nlohmann::json makeSessionListEntryJson(const Rendezvous::Session& session)
{
    nlohmann::json entry;
    entry["sessionId"] = session.sessionId;
    entry["hostAddress"] = session.hostAddress;
    entry["hostPort"] = session.hostPort;
    entry["hostLanAddress"] = session.hostLanAddress;
    entry["hostLanPort"] = session.hostLanPort;
    entry["gameName"] = session.gameName;
    entry["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                               session.createdAt.time_since_epoch())
                               .count();
    return entry;
}

bool isValidIp(const std::string& address)
{
    return !address.empty();
}

} // namespace

RendezvousServer::RendezvousServer(RendezvousConfig config)
    : config_(std::move(config))
    , sessionStore_(config_.sessionTimeout)
    , relay_(UdpRelayConfig{
          .bindAddress = config_.bindAddress,
          .portRangeStart = config_.relayPortRangeStart,
          .portRangeEnd = config_.relayPortRangeEnd,
          .allocationTimeout = config_.relayAllocationTimeout,
          .publicAddress = config_.relayPublicAddress,
      })
{
}

RendezvousServer::~RendezvousServer()
{
    stop();
}

bool RendezvousServer::run()
{
    if (running_.load())
    {
        spdlog::warn("RendezvousServer::run called while already running");
        return false;
    }

    server_ = std::make_unique<httplib::Server>();
    configureRoutes();

    running_.store(true);
    startTime_ = std::chrono::steady_clock::now();

    pruneThread_ = std::thread(&RendezvousServer::pruneLoop, this);

    if (config_.relayEnabled)
    {
        relay_.start();
    }

    spdlog::info(
        "RendezvousServer listening on {}:{} (metrics {}, relay {})",
        config_.bindAddress,
        config_.port,
        config_.metricsEnabled ? "enabled" : "disabled",
        config_.relayEnabled ? "enabled" : "disabled");

    const bool listenResult = server_->listen(config_.bindAddress.c_str(), config_.port);
    if (!listenResult)
    {
        spdlog::error(
            "RendezvousServer could not bind to {}:{}",
            config_.bindAddress,
            config_.port);
    }

    running_.store(false);
    relay_.stop();
    if (pruneThread_.joinable())
    {
        pruneThread_.join();
    }
    server_.reset();

    return listenResult;
}

void RendezvousServer::stop()
{
    const bool wasRunning = running_.exchange(false);
    if (!wasRunning)
    {
        return;
    }

    if (server_)
    {
        server_->stop();
    }

    relay_.stop();

    if (pruneThread_.joinable())
    {
        pruneThread_.join();
    }
}

void RendezvousServer::configureRoutes()
{
    server_->Get(
        "/healthz",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleHealthz(request, response);
        });

    server_->Get(
        "/readyz",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleReadyz(request, response);
        });

    server_->Get(
        "/status",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleStatus(request, response);
        });

    server_->Get(
        "/metrics",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleMetrics(request, response);
        });

    server_->Post(
        "/sessions",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleRegisterSession(request, response);
        });

    server_->Get(
        "/sessions",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleListSessions(request, response);
        });

    server_->Post(
        R"(/sessions/([^/]+)/heartbeat)",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleHeartbeat(request, response);
        });

    server_->Post(
        R"(/sessions/([^/]+)/punch)",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleRegisterPunchRequest(request, response);
        });

    server_->Post(
        R"(/sessions/([^/]+)/relay)",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleRegisterRelayRequest(request, response);
        });

    server_->Get(
        R"(/sessions/([^/]+)/requests)",
        [this](const httplib::Request& request, httplib::Response& response) {
            handleListRequests(request, response);
        });
}

void RendezvousServer::pruneLoop()
{
    const std::chrono::seconds interval = std::max(MinimumPruneInterval, config_.sessionTimeout / 4);
    while (running_.load())
    {
        sessionStore_.pruneExpired();
        std::this_thread::sleep_for(interval);
    }
}

void RendezvousServer::handleHealthz(const httplib::Request&, httplib::Response& response) const
{
    response.status = HttpStatus::Ok;
    response.set_content("ok\n", ContentTypeText);
}

void RendezvousServer::handleReadyz(const httplib::Request&, httplib::Response& response) const
{
    if (running_.load())
    {
        response.status = HttpStatus::Ok;
        response.set_content("ready\n", ContentTypeText);
    }
    else
    {
        response.status = HttpStatus::ServiceUnavailable;
        response.set_content("starting\n", ContentTypeText);
    }
}

void RendezvousServer::handleStatus(const httplib::Request&, httplib::Response& response) const
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const std::chrono::seconds uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime_);
    const std::vector<SessionStore::Session> sessionsSnapshot = sessionStore_.sessions();

    nlohmann::json payload;
    payload["uptimeSeconds"] = uptime.count();
    payload["sessionCount"] = static_cast<std::uint64_t>(sessionsSnapshot.size());

    response.status = HttpStatus::Ok;
    response.set_content(payload.dump(), ContentTypeJson);
}

void RendezvousServer::handleMetrics(const httplib::Request&, httplib::Response& response) const
{
    if (!config_.metricsEnabled)
    {
        response.status = HttpStatus::NotFound;
        response.set_content("metrics disabled\n", ContentTypeText);
        return;
    }

    const std::vector<SessionStore::Session> sessionsSnapshot = sessionStore_.sessions();
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const double uptimeSeconds = std::chrono::duration<double>(now - startTime_).count();

    std::ostringstream stream;
    stream << "# HELP rendezvous_sessions Number of active sessions\n";
    stream << "# TYPE rendezvous_sessions gauge\n";
    stream << "rendezvous_sessions " << sessionsSnapshot.size() << '\n';
    stream << "# HELP rendezvous_session_registrations_total Total session registrations\n";
    stream << "# TYPE rendezvous_session_registrations_total counter\n";
    stream << "rendezvous_session_registrations_total " << sessionRegistrationsTotal_.load() << '\n';
    stream << "# HELP rendezvous_heartbeats_total Total heartbeats received\n";
    stream << "# TYPE rendezvous_heartbeats_total counter\n";
    stream << "rendezvous_heartbeats_total " << heartbeatsTotal_.load() << '\n';
    stream << "# HELP rendezvous_punch_requests_total Total punch requests registered\n";
    stream << "# TYPE rendezvous_punch_requests_total counter\n";
    stream << "rendezvous_punch_requests_total " << punchRequestsTotal_.load() << '\n';
    stream << "# HELP rendezvous_relay_requests_total Total relay requests registered\n";
    stream << "# TYPE rendezvous_relay_requests_total counter\n";
    stream << "rendezvous_relay_requests_total " << relayRequestsTotal_.load() << '\n';
    stream << "# HELP rendezvous_process_uptime_seconds Process uptime in seconds\n";
    stream << "# TYPE rendezvous_process_uptime_seconds gauge\n";
    stream << "rendezvous_process_uptime_seconds " << uptimeSeconds << '\n';

    response.status = HttpStatus::Ok;
    response.set_content(stream.str(), ContentTypeText);
}

void RendezvousServer::handleRegisterSession(const httplib::Request& request, httplib::Response& response)
{
    try
    {
        const nlohmann::json body = nlohmann::json::parse(request.body);
        const bool hasHostPort = body.contains("hostPort") && body["hostPort"].is_number_integer();
        const bool hasGameName = body.contains("gameName") && body["gameName"].is_string();

        if (!hasHostPort || !hasGameName)
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("hostPort and gameName are required\n", ContentTypeText);
            return;
        }

        const std::string remoteAddress = request.remote_addr;
        if (!isValidIp(remoteAddress))
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("Unable to determine requester address\n", ContentTypeText);
            return;
        }

        Rendezvous::RegisterSessionRequest registerRequest{};
        registerRequest.hostPort = static_cast<uint16_t>(body["hostPort"].get<int>());
        registerRequest.gameName = body["gameName"].get<std::string>();

        if (body.contains("hostLanAddress") && body["hostLanAddress"].is_string())
        {
            registerRequest.hostLanAddress = body["hostLanAddress"].get<std::string>();
        }
        if (body.contains("hostLanPort") && body["hostLanPort"].is_number_integer())
        {
            registerRequest.hostLanPort = static_cast<uint16_t>(body["hostLanPort"].get<int>());
        }

        if (registerRequest.hostPort == 0)
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("Invalid port\n", ContentTypeText);
            return;
        }

        const SessionStore::Session session = sessionStore_.registerSession(
            remoteAddress,
            registerRequest.hostPort,
            registerRequest.hostLanAddress,
            registerRequest.hostLanPort,
            registerRequest.gameName);

        spdlog::info(
            "Registered session {} from {}:{} for '{}'",
            session.sessionId,
            session.hostAddress,
            session.hostPort,
            session.gameName);

        Rendezvous::RegisterSessionResponse registerResponse{};
        registerResponse.session = session;
        registerResponse.expiresIn = config_.sessionTimeout;

        sessionRegistrationsTotal_.fetch_add(1, std::memory_order_relaxed);

        response.status = HttpStatus::Created;
        response.set_content(makeRegisterSessionResponseJson(registerResponse).dump(), ContentTypeJson);
    }
    catch (const nlohmann::json::parse_error&)
    {
        response.status = HttpStatus::BadRequest;
        response.set_content("Invalid JSON\n", ContentTypeText);
    }
}

void RendezvousServer::handleRegisterPunchRequest(const httplib::Request& request, httplib::Response& response)
{
    try
    {
        const std::string sessionId = request.matches[1].str();

        const nlohmann::json body = nlohmann::json::parse(request.body);
        const bool hasClientPort = body.contains("clientPort") && body["clientPort"].is_number_integer();
        if (!hasClientPort)
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("clientPort is required\n", ContentTypeText);
            return;
        }

        const std::string remoteAddress = request.remote_addr;
        if (!isValidIp(remoteAddress))
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("Unable to determine requester address\n", ContentTypeText);
            return;
        }

        const uint16_t clientPort = static_cast<uint16_t>(body["clientPort"].get<int>());
        if (clientPort == 0)
        {
            response.status = HttpStatus::BadRequest;
            response.set_content("Invalid port\n", ContentTypeText);
            return;
        }

        const std::optional<Rendezvous::RegisterPunchResponse> punchResponse = sessionStore_.registerPunchRequest(
            sessionId,
            remoteAddress,
            clientPort);
        if (!punchResponse)
        {
            response.status = HttpStatus::NotFound;
            response.set_content("Unknown session\n", ContentTypeText);
            return;
        }

        punchRequestsTotal_.fetch_add(1, std::memory_order_relaxed);

        nlohmann::json responseJson;
        responseJson["requestId"] = punchResponse->requestId;

        response.status = HttpStatus::Created;
        response.set_content(responseJson.dump(), ContentTypeJson);
    }
    catch (const nlohmann::json::parse_error&)
    {
        response.status = HttpStatus::BadRequest;
        response.set_content("Invalid JSON\n", ContentTypeText);
    }
}

void RendezvousServer::handleRegisterRelayRequest(const httplib::Request& request, httplib::Response& response)
{
    const std::string sessionId = request.matches[1].str();

    if (!config_.relayEnabled)
    {
        response.status = HttpStatus::ServiceUnavailable;
        response.set_content("Relay is not enabled on this server\n", ContentTypeText);
        return;
    }

    const std::optional<RelayAllocation> allocation = relay_.allocate(sessionId);
    if (!allocation)
    {
        response.status = HttpStatus::ServiceUnavailable;
        response.set_content("No relay ports available\n", ContentTypeText);
        return;
    }

    const std::optional<Rendezvous::RegisterRelayResponse> relayResponse = sessionStore_.registerRelayRequest(
        sessionId,
        allocation->relayAddress,
        allocation->hostPort,
        allocation->clientPort);
    if (!relayResponse)
    {
        response.status = HttpStatus::NotFound;
        response.set_content("Unknown session\n", ContentTypeText);
        return;
    }

    relayRequestsTotal_.fetch_add(1, std::memory_order_relaxed);

    nlohmann::json responseJson;
    responseJson["requestId"] = relayResponse->requestId;
    responseJson["relayAddress"] = relayResponse->relayAddress;
    responseJson["relayHostPort"] = relayResponse->relayHostPort;
    responseJson["relayClientPort"] = relayResponse->relayClientPort;

    response.status = HttpStatus::Created;
    response.set_content(responseJson.dump(), ContentTypeJson);
}

void RendezvousServer::handleListRequests(const httplib::Request& request, httplib::Response& response)
{
    const std::string sessionId = request.matches[1].str();
    const std::optional<std::vector<Rendezvous::ConnectionRequestInfo>> pending =
        sessionStore_.consumeConnectionRequests(sessionId);
    if (!pending)
    {
        response.status = HttpStatus::NotFound;
        response.set_content("Unknown session\n", ContentTypeText);
        return;
    }

    nlohmann::json array = nlohmann::json::array();
    for (const Rendezvous::ConnectionRequestInfo& info : *pending)
    {
        nlohmann::json entry;
        entry["type"] = info.type;
        entry["requestId"] = info.requestId;
        if (info.type == "punch")
        {
            entry["clientAddress"] = info.clientAddress;
            entry["clientPort"] = info.clientPort;
        }
        else if (info.type == "relay")
        {
            entry["relayAddress"] = info.relayAddress;
            entry["relayHostPort"] = info.relayHostPort;
            entry["relayClientPort"] = info.relayClientPort;
        }
        array.push_back(std::move(entry));
    }

    response.status = HttpStatus::Ok;
    response.set_content(array.dump(), ContentTypeJson);
}

void RendezvousServer::handleListSessions(const httplib::Request& request, httplib::Response& response) const
{
    const std::string requesterAddress = request.remote_addr;
    const std::vector<SessionStore::Session> sessionsSnapshot = sessionStore_.sessions();
    nlohmann::json array = nlohmann::json::array();

    for (const SessionStore::Session& session : sessionsSnapshot)
    {
        nlohmann::json entry = makeSessionListEntryJson(session);

        // Only expose the host's LAN address to clients behind the same NAT
        if (session.hostAddress != requesterAddress)
        {
            entry.erase("hostLanAddress");
            entry.erase("hostLanPort");
        }

        array.push_back(std::move(entry));
    }

    spdlog::info("Returning {} sessions to {}", array.size(), requesterAddress);

    response.status = HttpStatus::Ok;
    response.set_content(array.dump(), ContentTypeJson);
}

void RendezvousServer::handleHeartbeat(const httplib::Request& request, httplib::Response& response)
{
    const std::string sessionId = request.matches[1].str();

    const bool updated = sessionStore_.heartbeat(sessionId);
    if (updated)
    {
        heartbeatsTotal_.fetch_add(1, std::memory_order_relaxed);
        response.status = HttpStatus::NoContent;
        spdlog::info("Heartbeat refreshed for session {}", sessionId);
    }
    else
    {
        response.status = HttpStatus::NotFound;
        response.set_content("Unknown session\n", ContentTypeText);
        spdlog::warn("Heartbeat received for unknown session {}", sessionId);
    }
}
