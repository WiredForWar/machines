#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

class StunClient
{
public:
    struct Result
    {
        std::string publicAddress{};
        uint16_t publicPort{};
    };

    StunClient(std::string serverHost, uint16_t serverPort);

    [[nodiscard]] std::optional<Result> queryFromSocket(int socketFd, std::chrono::milliseconds timeout) const;

private:
    std::string serverHost_{};
    uint16_t serverPort_{};
};
