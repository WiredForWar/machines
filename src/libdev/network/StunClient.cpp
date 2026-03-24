#include "network/StunClient.hpp"

#include "system/Endian.hpp"
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <random>

namespace
{
constexpr uint16_t StunBindingRequest = 0x0001;
constexpr uint16_t StunBindingResponse = 0x0101;
constexpr uint32_t StunMagicCookie = 0x2112A442;
constexpr size_t StunHeaderSize = 20;
constexpr uint8_t StunAddressFamilyIPv4 = 0x01;

enum class StunAttributeType : uint16_t
{
    MappedAddress = 0x0001,
    XorMappedAddress = 0x0020,
};

struct StunHeader
{
    uint16_t type;
    uint16_t length;
    uint32_t magicCookie;
    std::array<uint8_t, 12> transactionId;
};

static_assert(sizeof(StunHeader) == StunHeaderSize, "Unexpected STUN header size");

struct StunAttributeHeader
{
    uint16_t type;
    uint16_t length;
};

static_assert(sizeof(StunAttributeHeader) == 4, "Unexpected STUN attribute header size");

struct StunMappedAddressAttribute
{
    uint8_t unused;
    uint8_t family;
    uint16_t port;
    uint32_t address;
};

static_assert(sizeof(StunMappedAddressAttribute) == 8, "Unexpected STUN mapped address size");

bool parseAddressAttribute(
    StunAttributeType attributeType,
    const uint8_t* attributeData,
    uint16_t attributeLength,
    const uint8_t transactionId[12],
    StunClient::Result* outResult)
{
    if (attributeLength < sizeof(StunMappedAddressAttribute))
        return false;

    StunMappedAddressAttribute attribute{};
    std::memcpy(&attribute, attributeData, sizeof(attribute));

    if (attribute.family != StunAddressFamilyIPv4)
        return false;

    uint16_t port = System::fromBigEndian(attribute.port);
    uint32_t addressHostOrder = System::fromBigEndian(attribute.address);

    if (attributeType == StunAttributeType::XorMappedAddress)
    {
        port ^= static_cast<uint16_t>(StunMagicCookie >> 16U);

        if (attributeLength >= 8)
        {
            addressHostOrder ^= StunMagicCookie;
        }

        // XOR the remaining 96 bits when transaction ID is available (IPv6). Not needed for IPv4.
        (void)transactionId;
    }

    in_addr ipv4{};
    ipv4.s_addr = htonl(addressHostOrder);

    char addressBuffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &ipv4, addressBuffer, sizeof(addressBuffer)) == nullptr)
        return false;

    outResult->publicAddress = addressBuffer;
    outResult->publicPort = port;
    return true;
}

} // namespace

StunClient::StunClient(std::string serverHost, uint16_t serverPort)
    : serverHost_(std::move(serverHost))
    , serverPort_(serverPort)
{
}

std::optional<StunClient::Result> StunClient::queryFromSocket(int socketFd, std::chrono::milliseconds timeout) const
{
    spdlog::warn(
        "StunClient: queryFromSocket with params: host='{}', port={}, socketFd={}",
        serverHost_,
        serverPort_,
        socketFd);

    if (socketFd < 0 || serverHost_.empty() || serverPort_ == 0)
    {
        spdlog::warn("StunClient: queryFromSocket invalid configuration");
        return std::nullopt;
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    addrinfo* resolved = nullptr;
    const std::string portString = std::to_string(serverPort_);
    const int resolveResult = getaddrinfo(serverHost_.c_str(), portString.c_str(), &hints, &resolved);
    if (resolveResult != 0)
    {
        spdlog::warn(
            "StunClient: getaddrinfo failed for {}:{} ({})",
            serverHost_,
            serverPort_,
            gai_strerror(resolveResult));
        return std::nullopt;
    }

    const addrinfo* stunEndpoint = resolved;
    if (stunEndpoint == nullptr)
    {
        freeaddrinfo(resolved);
        spdlog::warn("StunClient: getaddrinfo returned no endpoints for {}:{}", serverHost_, serverPort_);
        return std::nullopt;
    }

    StunHeader requestHeader{};
    requestHeader.type = System::toBigEndian<uint16_t>(StunBindingRequest);
    requestHeader.length = 0;
    requestHeader.magicCookie = System::toBigEndian<uint32_t>(StunMagicCookie);

    std::array<uint8_t, 12> transactionId{};
    std::random_device rd;
    for (uint8_t& byte : transactionId)
    {
        byte = static_cast<uint8_t>(rd());
    }
    requestHeader.transactionId = transactionId;

    std::array<uint8_t, sizeof(StunHeader)> request{};
    std::memcpy(request.data(), &requestHeader, sizeof(requestHeader));

    const ssize_t sent = ::sendto(socketFd, request.data(), request.size(), 0, stunEndpoint->ai_addr, stunEndpoint->ai_addrlen);
    if (sent < 0)
    {
        const int sendErrno = errno;
        freeaddrinfo(resolved);
        spdlog::warn("StunClient: sendto() failed: {}", std::strerror(sendErrno));
        return std::nullopt;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(socketFd, &readSet);

    timeval tv{};
    tv.tv_sec = static_cast<time_t>(timeout.count() / 1000);
    tv.tv_usec = static_cast<suseconds_t>((timeout.count() % 1000) * 1000);

    const int ready = ::select(socketFd + 1, &readSet, nullptr, nullptr, &tv);
    if (ready <= 0)
    {
        freeaddrinfo(resolved);
        if (ready == 0)
        {
            spdlog::warn("StunClient: queryFromSocket timed out waiting for response");
        }
        else
        {
            const int selectErrno = errno;
            spdlog::warn("StunClient: select() failed: {}", std::strerror(selectErrno));
        }
        return std::nullopt;
    }

    std::array<uint8_t, 512> response{};
    sockaddr_storage from{};
    socklen_t fromLen = sizeof(from);
    const ssize_t received = ::recvfrom(socketFd, response.data(), response.size(), 0, reinterpret_cast<sockaddr*>(&from), &fromLen);
    freeaddrinfo(resolved);

    if (received < static_cast<ssize_t>(sizeof(StunHeader)))
    {
        if (received < 0)
        {
            const int recvErrno = errno;
            spdlog::warn("StunClient: recvfrom() failed: {}", std::strerror(recvErrno));
        }
        else
        {
            spdlog::warn("StunClient: Received truncated STUN response ({} bytes)", received);
        }
        return std::nullopt;
    }

    StunHeader responseHeader{};
    std::size_t offset = 0;
    std::memcpy(&responseHeader, response.data() + offset, sizeof(StunHeader));
    offset += sizeof(StunHeader);

    if (System::fromBigEndian(responseHeader.type) != StunBindingResponse)
    {
        spdlog::warn("StunClient: Unexpected STUN response type 0x{:04x}", System::fromBigEndian(responseHeader.type));
        return std::nullopt;
    }

    if (System::fromBigEndian(responseHeader.magicCookie) != StunMagicCookie)
    {
        spdlog::warn("StunClient: Invalid STUN magic cookie in response");
        return std::nullopt;
    }

    if (responseHeader.transactionId != requestHeader.transactionId)
    {
        spdlog::warn("StunClient: Mismatched STUN transaction ID in response");
        return std::nullopt;
    }

    const uint16_t messageLength = System::fromBigEndian(responseHeader.length);
    const std::size_t payloadAvailable = static_cast<std::size_t>(received) - sizeof(StunHeader);
    const std::size_t payloadSize = std::min<std::size_t>(messageLength, payloadAvailable);
    std::size_t remaining = payloadSize;

    while (remaining >= sizeof(StunAttributeHeader))
    {
        StunAttributeHeader attributeHeader{};
        std::memcpy(&attributeHeader, response.data() + offset, sizeof(attributeHeader));
        const uint16_t attributeTypeNetwork = System::fromBigEndian(attributeHeader.type);
        const uint16_t attributeLength = System::fromBigEndian(attributeHeader.length);

        offset += sizeof(StunAttributeHeader);
        remaining -= sizeof(StunAttributeHeader);

        if (attributeLength > remaining)
        {
            break;
        }

        const StunAttributeType attributeType = static_cast<StunAttributeType>(attributeTypeNetwork);
        if (attributeType == StunAttributeType::XorMappedAddress || attributeType == StunAttributeType::MappedAddress)
        {
            const uint8_t* attributeData = response.data() + offset;
            Result result{};
            if (parseAddressAttribute(attributeType, attributeData, attributeLength, transactionId.data(), &result))
            {
                spdlog::info("StunClient: Mapped public endpoint {}:{}", result.publicAddress, result.publicPort);
                return result;
            }
        }

        const std::size_t paddedLength = (attributeLength + 3) & ~std::size_t(3);
        const std::size_t advance = std::min<std::size_t>(paddedLength, remaining);
        offset += advance;
        remaining -= advance;
    }

    spdlog::warn("StunClient: STUN response missing mapped address attribute");
    return std::nullopt;
}
