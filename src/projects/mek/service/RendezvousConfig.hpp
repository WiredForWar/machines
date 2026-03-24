#pragma once

#include <chrono>
#include <cstdint>
#include <string>

struct RendezvousConfig
{
    std::string bindAddress{};
    uint16_t port{};
    std::chrono::seconds sessionTimeout{};
};
