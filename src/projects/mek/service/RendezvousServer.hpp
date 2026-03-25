#pragma once

#include "service/RendezvousConfig.hpp"
#include "service/SessionStore.hpp"
#include "service/UdpRelay.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace httplib
{
class Server;
struct Request;
struct Response;
} // namespace httplib

class RendezvousServer
{
public:
    explicit RendezvousServer(RendezvousConfig config);
    ~RendezvousServer();

    bool run();
    void stop();

private:
    void configureRoutes();
    void pruneLoop();

    void handleHealthz(const httplib::Request& request, httplib::Response& response) const;
    void handleReadyz(const httplib::Request& request, httplib::Response& response) const;
    void handleStatus(const httplib::Request& request, httplib::Response& response) const;
    void handleMetrics(const httplib::Request& request, httplib::Response& response) const;
    void handleRegisterSession(const httplib::Request& request, httplib::Response& response);
    void handleListSessions(const httplib::Request& request, httplib::Response& response) const;
    void handleHeartbeat(const httplib::Request& request, httplib::Response& response);
    void handleRegisterPunchRequest(const httplib::Request& request, httplib::Response& response);
    void handleRegisterRelayRequest(const httplib::Request& request, httplib::Response& response);
    void handleListRequests(const httplib::Request& request, httplib::Response& response);

    RendezvousConfig config_;
    SessionStore sessionStore_;
    UdpRelay relay_;
    std::atomic<bool> running_{false};
    std::unique_ptr<httplib::Server> server_{};
    std::thread pruneThread_{};
    std::chrono::steady_clock::time_point startTime_{};

    // Metrics counters
    std::atomic<uint64_t> sessionRegistrationsTotal_{};
    std::atomic<uint64_t> heartbeatsTotal_{};
    std::atomic<uint64_t> punchRequestsTotal_{};
    std::atomic<uint64_t> relayRequestsTotal_{};
};
