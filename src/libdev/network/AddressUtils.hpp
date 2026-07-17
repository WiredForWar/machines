#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

std::string makeAddress(std::string_view ip, uint16_t port);
std::string_view getHost(const std::string_view& addressStr);
std::optional<uint16_t> getPort(const std::string_view& addressStr);
