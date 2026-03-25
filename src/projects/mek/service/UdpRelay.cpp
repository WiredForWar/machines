#include "service/UdpRelay.hpp"

#include <spdlog/spdlog.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <sstream>

namespace
{
constexpr int MaxPacketSize = 2048;
constexpr std::chrono::milliseconds PollTimeout{50};
} // namespace

UdpRelay::UdpRelay(UdpRelayConfig config)
    : config_(std::move(config))
    , nextPort_(config_.portRangeStart)
{
    if (config_.publicAddress.empty())
    {
        config_.publicAddress = config_.bindAddress;
    }
}

UdpRelay::~UdpRelay()
{
    stop();
}

bool UdpRelay::start()
{
    if (running_.load())
    {
        return false;
    }

    running_.store(true);
    relayThread_ = std::thread(&UdpRelay::relayLoop, this);

    spdlog::info(
        "UdpRelay started on {} ports {}-{} (public: {})",
        config_.bindAddress,
        config_.portRangeStart,
        config_.portRangeEnd,
        config_.publicAddress);
    return true;
}

void UdpRelay::stop()
{
    const bool wasRunning = running_.exchange(false);
    if (!wasRunning)
    {
        return;
    }

    if (relayThread_.joinable())
    {
        relayThread_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (SocketPair& pair : pairs_)
    {
        closePair(pair);
    }
    pairs_.clear();
}

std::optional<RelayAllocation> UdpRelay::allocate(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Find two consecutive free ports
    const uint16_t rangeSize = config_.portRangeEnd - config_.portRangeStart + 1;
    if (rangeSize < 2)
    {
        spdlog::warn("UdpRelay: port range too small for allocation");
        return std::nullopt;
    }

    // Try up to rangeSize/2 attempts to find a free pair
    const uint16_t maxAttempts = rangeSize / 2;
    for (uint16_t attempt = 0; attempt < maxAttempts; ++attempt)
    {
        // Wrap around within range
        if (nextPort_ + 1 > config_.portRangeEnd)
        {
            nextPort_ = config_.portRangeStart;
        }

        const uint16_t hostPort = nextPort_;
        const uint16_t clientPort = nextPort_ + 1;
        nextPort_ += 2;

        // Check if these ports are already in use by an existing pair
        bool inUse = false;
        for (const SocketPair& existing : pairs_)
        {
            if (existing.hostPort == hostPort || existing.clientPort == clientPort
                || existing.hostPort == clientPort || existing.clientPort == hostPort)
            {
                inUse = true;
                break;
            }
        }
        if (inUse)
        {
            continue;
        }

        const int hostFd = createBoundSocket(hostPort);
        if (hostFd < 0)
        {
            continue;
        }

        const int clientFd = createBoundSocket(clientPort);
        if (clientFd < 0)
        {
            close(hostFd);
            continue;
        }

        SocketPair pair{};
        {
            std::ostringstream stream;
            stream << std::hex << std::uppercase << nextAllocationId_++;
            pair.allocationId = stream.str();
        }
        pair.sessionId = sessionId;
        pair.hostFd = hostFd;
        pair.clientFd = clientFd;
        pair.hostPort = hostPort;
        pair.clientPort = clientPort;
        pair.lastActivity = std::chrono::steady_clock::now();

        RelayAllocation allocation{};
        allocation.allocationId = pair.allocationId;
        allocation.relayAddress = config_.publicAddress;
        allocation.hostPort = hostPort;
        allocation.clientPort = clientPort;

        spdlog::info(
            "UdpRelay: allocated {} for session {} (host port {}, client port {})",
            pair.allocationId,
            sessionId,
            hostPort,
            clientPort);

        pairs_.push_back(std::move(pair));
        return allocation;
    }

    spdlog::warn("UdpRelay: no free ports available for allocation");
    return std::nullopt;
}

void UdpRelay::relayLoop()
{
    std::vector<uint8_t> buffer(MaxPacketSize);

    while (running_.load())
    {
        std::vector<struct pollfd> fds;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            fds.reserve(pairs_.size() * 2);
            for (const SocketPair& pair : pairs_)
            {
                fds.push_back({pair.hostFd, POLLIN, 0});
                fds.push_back({pair.clientFd, POLLIN, 0});
            }
        }

        if (fds.empty())
        {
            std::this_thread::sleep_for(PollTimeout);
            pruneExpired();
            continue;
        }

        const int ready = poll(fds.data(), fds.size(), static_cast<int>(PollTimeout.count()));
        if (ready <= 0)
        {
            pruneExpired();
            continue;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        for (size_t i = 0; i < pairs_.size(); ++i)
        {
            SocketPair& pair = pairs_[i];
            const size_t fdIndex = i * 2;

            // Data on host-side socket → forward to client remote
            if ((fds[fdIndex].revents & POLLIN) != 0)
            {
                struct sockaddr_storage sender{};
                socklen_t senderLen = sizeof(sender);
                const ssize_t n = recvfrom(
                    pair.hostFd,
                    buffer.data(),
                    buffer.size(),
                    0,
                    reinterpret_cast<struct sockaddr*>(&sender),
                    &senderLen);
                if (n > 0)
                {
                    // Learn the host's remote address on first packet
                    if (pair.hostRemoteLen == 0)
                    {
                        pair.hostRemote = sender;
                        pair.hostRemoteLen = senderLen;
                    }
                    pair.lastActivity = now;

                    // Forward to client remote if known
                    if (pair.clientRemoteLen > 0)
                    {
                        sendto(
                            pair.clientFd,
                            buffer.data(),
                            static_cast<size_t>(n),
                            0,
                            reinterpret_cast<struct sockaddr*>(&pair.clientRemote),
                            pair.clientRemoteLen);
                    }
                }
            }

            // Data on client-side socket → forward to host remote
            if ((fds[fdIndex + 1].revents & POLLIN) != 0)
            {
                struct sockaddr_storage sender{};
                socklen_t senderLen = sizeof(sender);
                const ssize_t n = recvfrom(
                    pair.clientFd,
                    buffer.data(),
                    buffer.size(),
                    0,
                    reinterpret_cast<struct sockaddr*>(&sender),
                    &senderLen);
                if (n > 0)
                {
                    // Learn the client's remote address on first packet
                    if (pair.clientRemoteLen == 0)
                    {
                        pair.clientRemote = sender;
                        pair.clientRemoteLen = senderLen;
                    }
                    pair.lastActivity = now;

                    // Forward to host remote if known
                    if (pair.hostRemoteLen > 0)
                    {
                        sendto(
                            pair.hostFd,
                            buffer.data(),
                            static_cast<size_t>(n),
                            0,
                            reinterpret_cast<struct sockaddr*>(&pair.hostRemote),
                            pair.hostRemoteLen);
                    }
                }
            }
        }

        pruneExpiredLocked();
    }
}

void UdpRelay::pruneExpired()
{
    std::lock_guard<std::mutex> lock(mutex_);
    pruneExpiredLocked();
}

void UdpRelay::pruneExpiredLocked()
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto iterator = pairs_.begin();
    while (iterator != pairs_.end())
    {
        if (now - iterator->lastActivity > config_.allocationTimeout)
        {
            spdlog::info("UdpRelay: pruning expired allocation {}", iterator->allocationId);
            closePair(*iterator);
            iterator = pairs_.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

void UdpRelay::closePair(SocketPair& pair)
{
    if (pair.hostFd >= 0)
    {
        close(pair.hostFd);
        pair.hostFd = -1;
    }
    if (pair.clientFd >= 0)
    {
        close(pair.clientFd);
        pair.clientFd = -1;
    }
}

int UdpRelay::createBoundSocket(uint16_t port) const
{
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        spdlog::warn("UdpRelay: socket() failed: {}", strerror(errno));
        return -1;
    }

    // Set non-blocking
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
    {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    // Allow port reuse
    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, config_.bindAddress.c_str(), &addr.sin_addr);

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        spdlog::warn("UdpRelay: bind() to port {} failed: {}", port, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}
