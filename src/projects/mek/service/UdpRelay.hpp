#pragma once

#include <sys/socket.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

struct RelayAllocation
{
    std::string allocationId{};
    // The two endpoints that external peers connect to
    std::string relayAddress{};
    uint16_t hostPort{};
    uint16_t clientPort{};
};

struct UdpRelayConfig
{
    std::string bindAddress{"0.0.0.0"};
    // Port range for relay allocations (inclusive)
    uint16_t portRangeStart{40000};
    uint16_t portRangeEnd{40999};
    // How long an idle allocation lives before being reclaimed
    std::chrono::seconds allocationTimeout{60};
    // The public address clients see (may differ from bindAddress)
    std::string publicAddress{};
};

// Manages a pool of UDP relay socket pairs. Each allocation creates two
// sockets; one for the host and one for the client. Traffic arriving on
// either socket is forwarded to the other.
//
// Thread-safety: allocate() may be called from any thread. The relay
// forwarding and pruning run on a dedicated thread.
class UdpRelay
{
public:
    explicit UdpRelay(UdpRelayConfig config);
    ~UdpRelay();

    UdpRelay(const UdpRelay&) = delete;
    UdpRelay& operator=(const UdpRelay&) = delete;

    bool start();
    void stop();

    // Allocate a relay pair. Returns nullopt if no ports are available.
    std::optional<RelayAllocation> allocate(const std::string& sessionId);

    const UdpRelayConfig& config() const { return config_; }

private:
    struct SocketPair
    {
        std::string allocationId{};
        std::string sessionId{};
        int hostFd{-1};
        int clientFd{-1};
        uint16_t hostPort{};
        uint16_t clientPort{};
        // Learned remote addresses (set on first packet from each side)
        struct sockaddr_storage hostRemote{};
        socklen_t hostRemoteLen{};
        struct sockaddr_storage clientRemote{};
        socklen_t clientRemoteLen{};
        std::chrono::steady_clock::time_point lastActivity{};
    };

    void relayLoop();
    void pruneExpired();
    void pruneExpiredLocked();
    void closePair(SocketPair& pair);
    int createBoundSocket(uint16_t port) const;

    UdpRelayConfig config_{};
    std::atomic<bool> running_{};
    std::thread relayThread_{};

    mutable std::mutex mutex_{};
    std::vector<SocketPair> pairs_{};
    uint64_t nextAllocationId_{1};
    uint16_t nextPort_{};
};
