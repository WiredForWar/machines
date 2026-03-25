#include "rendezvous/RendezvousClient.hpp"

#include "rendezvous/HttpStatus.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <utility>

namespace Rendezvous
{
namespace
{
constexpr const char* ContentTypeJson = "application/json";
} // namespace

Client::Client(RendezvousClientConfig config)
    : config_(std::move(config))
{
    if (config_.apiPathPrefix.empty())
    {
        config_.apiPathPrefix = "/";
    }

    if (config_.apiPathPrefix.front() != '/')
    {
        config_.apiPathPrefix.insert(config_.apiPathPrefix.begin(), '/');
    }

    if (config_.apiPathPrefix.length() > 1 && config_.apiPathPrefix.back() == '/')
    {
        config_.apiPathPrefix.pop_back();
    }

    if (config_.networkTimeout.count() == 0)
    {
        config_.networkTimeout = std::chrono::seconds(5);
    }
}

bool Client::registerSession(const RegisterSessionRequest& request, RegisterSessionResponse* response) const
{
    if (response == nullptr)
    {
        return false;
    }

    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return false;
    }

    nlohmann::json payload;
    payload["hostPort"] = request.hostPort;
    payload["gameName"] = request.gameName;
    if (!request.hostLanAddress.empty())
    {
        payload["hostLanAddress"] = request.hostLanAddress;
        payload["hostLanPort"] = request.hostLanPort;
    }

    const std::string path = makePath("/sessions");
    const httplib::Result result = httpClient->Post(path.c_str(), payload.dump(), ContentTypeJson);
    if (!result)
    {
        spdlog::warn("Rendezvous client: registerSession request failed");
        return false;
    }

    if (result->status != HttpStatus::Created)
    {
        spdlog::warn("Rendezvous client: registerSession unexpected status {}", result->status);
        return false;
    }

    const std::optional<RegisterSessionResponse> parsed = parseRegisterResponse(result->body);
    if (!parsed)
    {
        return false;
    }

    *response = *parsed;
    return true;
}

bool Client::sendHeartbeat(const std::string& sessionId) const
{
    if (sessionId.empty())
    {
        return false;
    }

    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return false;
    }

    const std::string path = makePath("/sessions/" + sessionId + "/heartbeat");
    const httplib::Result result = httpClient->Post(path.c_str(), "", ContentTypeJson);
    if (!result)
    {
        spdlog::warn("Rendezvous client: heartbeat request failed");
        return false;
    }

    if (result->status == HttpStatus::NoContent)
    {
        return true;
    }

    if (result->status == HttpStatus::NotFound)
    {
        spdlog::info("Rendezvous client: heartbeat rejected for unknown session");
        return false;
    }

    spdlog::warn("Rendezvous client: heartbeat unexpected status {}", result->status);
    return false;
}

bool Client::registerPunchRequest(const std::string& sessionId, const RegisterPunchRequest& request, RegisterPunchResponse* response) const
{
    if (response == nullptr)
    {
        return false;
    }

    if (sessionId.empty() || request.clientPort == 0)
    {
        return false;
    }

    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return false;
    }

    nlohmann::json payload;
    payload["clientPort"] = request.clientPort;

    const std::string path = makePath("/sessions/" + sessionId + "/punch");
    const httplib::Result result = httpClient->Post(path.c_str(), payload.dump(), ContentTypeJson);
    if (!result)
    {
        spdlog::warn("Rendezvous client: registerPunch request failed");
        return false;
    }

    if (result->status != HttpStatus::Created)
    {
        spdlog::warn("Rendezvous client: registerPunch unexpected status {}", result->status);
        return false;
    }

    const std::optional<RegisterPunchResponse> parsed = parseRegisterPunchResponse(result->body);
    if (!parsed)
    {
        return false;
    }

    *response = *parsed;
    return true;
}

bool Client::registerRelayRequest(const std::string& sessionId, RegisterRelayResponse* response) const
{
    if (response == nullptr)
    {
        return false;
    }

    if (sessionId.empty())
    {
        return false;
    }

    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return false;
    }

    const std::string path = makePath("/sessions/" + sessionId + "/relay");
    const httplib::Result result = httpClient->Post(path.c_str(), "", ContentTypeJson);
    if (!result)
    {
        spdlog::warn("Rendezvous client: registerRelay request failed");
        return false;
    }

    if (result->status != HttpStatus::Created)
    {
        spdlog::warn("Rendezvous client: registerRelay unexpected status {}", result->status);
        return false;
    }

    const std::optional<RegisterRelayResponse> parsed = parseRegisterRelayResponse(result->body);
    if (!parsed)
    {
        return false;
    }

    *response = *parsed;
    return true;
}

std::optional<std::vector<ConnectionRequestInfo>> Client::listRequests(const std::string& sessionId) const
{
    if (sessionId.empty())
    {
        return {};
    }

    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return {};
    }

    const std::string path = makePath("/sessions/" + sessionId + "/requests");
    const httplib::Result result = httpClient->Get(path.c_str());
    if (!result)
    {
        spdlog::warn("Rendezvous client: listRequests request failed");
        return {};
    }

    if (result->status != HttpStatus::Ok)
    {
        spdlog::warn("Rendezvous client: listRequests unexpected status {}", result->status);
        return {};
    }

    return parseConnectionRequestList(result->body);
}

std::optional<std::vector<Session>> Client::listSessions() const
{
    std::unique_ptr<httplib::ClientImpl> httpClient = createHttpClient();
    if (!httpClient)
    {
        return {};
    }

    const std::string path = makePath("/sessions");
    const httplib::Result result = httpClient->Get(path.c_str());
    if (!result)
    {
        spdlog::warn("Rendezvous client: listSessions request failed");
        return {};
    }

    if (result->status != HttpStatus::Ok)
    {
        spdlog::warn("Rendezvous client: listSessions unexpected status {}", result->status);
        return {};
    }

    return parseSessionList(result->body);
}

std::unique_ptr<httplib::ClientImpl> Client::createHttpClient() const
{
    std::unique_ptr<httplib::ClientImpl> client;

    if (config_.useHttps)
    {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto sslClient = std::make_unique<httplib::SSLClient>(config_.host, static_cast<int>(config_.port));
        sslClient->enable_server_certificate_verification(false);
        client = std::unique_ptr<httplib::ClientImpl>(sslClient.release());
#else
        spdlog::error("Rendezvous client: HTTPS requested but cpp-httplib lacks OpenSSL support");
        return nullptr;
#endif
    }
    else
    {
        client = std::make_unique<httplib::ClientImpl>(config_.host, static_cast<int>(config_.port));
    }

    const auto timeout = (config_.networkTimeout.count() > 0)
        ? config_.networkTimeout
        : std::chrono::seconds(5);
    client->set_connection_timeout(timeout);
    client->set_read_timeout(timeout);
    client->set_write_timeout(timeout);
    return client;
}

std::string Client::makePath(const std::string& suffix) const
{
    if (config_.apiPathPrefix == "/")
    {
        return suffix;
    }

    return config_.apiPathPrefix + suffix;
}

std::optional<RegisterSessionResponse> Client::parseRegisterResponse(const std::string& body)
{
    try
    {
        const nlohmann::json json = nlohmann::json::parse(body);
        RegisterSessionResponse response{};
        response.session.sessionId = json.value("sessionId", std::string{});
        response.session.hostAddress = json.value("hostAddress", std::string{});
        response.session.hostPort = static_cast<uint16_t>(json.value("hostPort", 0));
        response.session.hostLanAddress = json.value("hostLanAddress", std::string{});
        response.session.hostLanPort = static_cast<uint16_t>(json.value("hostLanPort", 0));
        response.session.gameName = json.value("gameName", std::string{});
        response.expiresIn = std::chrono::seconds(json.value("expiresInSeconds", 0));
        return response;
    }
    catch (const nlohmann::json::parse_error&)
    {
        spdlog::warn("Rendezvous client: unable to parse registerSession response");
        return {};
    }
}

std::optional<std::vector<Session>> Client::parseSessionList(const std::string& body)
{
    try
    {
        const nlohmann::json array = nlohmann::json::parse(body);
        if (!array.is_array())
        {
            return {};
        }

        std::vector<Session> sessions;
        sessions.reserve(array.size());

        for (nlohmann::json::const_iterator it = array.begin(); it != array.end(); ++it)
        {
            const nlohmann::json& entry = *it;
            Session session;
            session.sessionId = entry.value("sessionId", std::string{});
            session.hostAddress = entry.value("hostAddress", std::string{});
            session.hostPort = static_cast<uint16_t>(entry.value("hostPort", 0));
            session.hostLanAddress = entry.value("hostLanAddress", std::string{});
            session.hostLanPort = static_cast<uint16_t>(entry.value("hostLanPort", 0));
            session.gameName = entry.value("gameName", std::string{});
            sessions.push_back(session);
        }

        return sessions;
    }
    catch (const nlohmann::json::parse_error&)
    {
        spdlog::warn("Rendezvous client: unable to parse session list response");
        return {};
    }
}

std::optional<RegisterPunchResponse> Client::parseRegisterPunchResponse(const std::string& body)
{
    try
    {
        const nlohmann::json json = nlohmann::json::parse(body);
        RegisterPunchResponse response{};
        response.requestId = json.value("requestId", std::string{});
        if (response.requestId.empty())
        {
            return {};
        }

        return response;
    }
    catch (const nlohmann::json::parse_error&)
    {
        spdlog::warn("Rendezvous client: unable to parse registerPunch response");
        return {};
    }
}

std::optional<RegisterRelayResponse> Client::parseRegisterRelayResponse(const std::string& body)
{
    try
    {
        const nlohmann::json json = nlohmann::json::parse(body);
        RegisterRelayResponse response{};
        response.requestId = json.value("requestId", std::string{});
        response.relayAddress = json.value("relayAddress", std::string{});
        response.relayHostPort = static_cast<uint16_t>(json.value("relayHostPort", 0));
        response.relayClientPort = static_cast<uint16_t>(json.value("relayClientPort", 0));

        if (response.requestId.empty() || response.relayAddress.empty()
            || response.relayHostPort == 0 || response.relayClientPort == 0)
        {
            return {};
        }

        return response;
    }
    catch (const nlohmann::json::parse_error&)
    {
        spdlog::warn("Rendezvous client: unable to parse registerRelay response");
        return {};
    }
}

std::optional<std::vector<ConnectionRequestInfo>> Client::parseConnectionRequestList(const std::string& body)
{
    try
    {
        const nlohmann::json array = nlohmann::json::parse(body);
        if (!array.is_array())
        {
            return {};
        }

        std::vector<ConnectionRequestInfo> requests;
        requests.reserve(array.size());

        for (nlohmann::json::const_iterator it = array.begin(); it != array.end(); ++it)
        {
            const nlohmann::json& entry = *it;
            ConnectionRequestInfo info{};
            info.type = entry.value("type", std::string{});
            info.requestId = entry.value("requestId", std::string{});

            if (info.type == "punch")
            {
                info.clientAddress = entry.value("clientAddress", std::string{});
                info.clientPort = static_cast<uint16_t>(entry.value("clientPort", 0));
            }
            else if (info.type == "relay")
            {
                info.relayAddress = entry.value("relayAddress", std::string{});
                info.relayHostPort = static_cast<uint16_t>(entry.value("relayHostPort", 0));
                info.relayClientPort = static_cast<uint16_t>(entry.value("relayClientPort", 0));
            }

            if (!info.requestId.empty())
            {
                requests.push_back(std::move(info));
            }
        }

        return requests;
    }
    catch (const nlohmann::json::parse_error&)
    {
        spdlog::warn("Rendezvous client: unable to parse connection request list response");
        return {};
    }
}

} // namespace Rendezvous
