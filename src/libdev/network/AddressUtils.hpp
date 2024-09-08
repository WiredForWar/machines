#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

std::string_view getIp(const std::string& addressStr);
std::optional<uint16_t> getPort(const std::string& addressStr);
