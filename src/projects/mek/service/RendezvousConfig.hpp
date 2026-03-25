#pragma once

#include <chrono>
#include <cstdint>
#include <string>

struct RendezvousConfig
{
    std::string bindAddress{};
    uint16_t port{};
    bool metricsEnabled{};
    std::chrono::seconds sessionTimeout{};
    bool relayEnabled{};
    std::string relayPublicAddress{};
    uint16_t relayPortRangeStart{40000};
    uint16_t relayPortRangeEnd{40999};
    std::chrono::seconds relayAllocationTimeout{60};
};
