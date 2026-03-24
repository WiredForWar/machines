#include "rendezvous/RendezvousClient.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{

void initLogging()
{
    try
    {
        std::shared_ptr<spdlog::logger> logger = spdlog::get("mek");
        if (!logger)
        {
            logger = spdlog::stderr_color_mt("mek");
        }

        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Failed to initialize logging: " << ex.what() << std::endl;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    initLogging();

    std::string serverAddress{"127.0.0.1"};
    uint16_t serverPort = 8080;
    if (argc >= 2)
    {
        serverAddress = argv[1];
    }
    if (argc >= 3)
    {
        serverPort = static_cast<uint16_t>(std::stoi(argv[2]));
    }

    Rendezvous::RendezvousClientConfig clientConfig{
        .host = serverAddress,
        .port = serverPort,
        .useHttps = false,
        .apiPathPrefix = "/",
    };
    Rendezvous::Client rendezvousClient(std::move(clientConfig));

    Rendezvous::RegisterSessionRequest registerRequest{};
    registerRequest.hostPort = 5555;
    registerRequest.gameName = "Test Session";

    spdlog::info("Registering session at {}:{}", serverAddress, serverPort);
    Rendezvous::RegisterSessionResponse registerResponse{};
    if (!rendezvousClient.registerSession(registerRequest, &registerResponse))
    {
        spdlog::error("Failed to register session");
        return 1;
    }

    spdlog::info(
        "Session {} registered at {}:{} (expires in {}s)",
        registerResponse.session.sessionId,
        registerResponse.session.hostAddress,
        registerResponse.session.hostPort,
        registerResponse.expiresIn.count());

    const std::string& sessionId = registerResponse.session.sessionId;
    if (rendezvousClient.sendHeartbeat(sessionId))
    {
        spdlog::info("Heartbeat accepted for session {}", sessionId);
    }
    else
    {
        spdlog::warn("Heartbeat failed for session {}", sessionId);
    }

    spdlog::info("Listing sessions");
    const std::optional<std::vector<Rendezvous::Session>> sessions = rendezvousClient.listSessions();
    if (!sessions)
    {
        spdlog::error("Failed to fetch sessions");
        return 1;
    }

    spdlog::info("{} sessions returned", sessions->size());
    for (const Rendezvous::Session& session : *sessions)
    {
        spdlog::info("- {} {}:{} '{}'", session.sessionId, session.hostAddress, session.hostPort, session.gameName);
    }

    return 0;
}
