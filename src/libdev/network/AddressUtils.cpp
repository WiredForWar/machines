#include <optional>
#include <string_view>
#include <algorithm>
#include <charconv>

#include "AddressUtils.hpp"

std::string_view getIp(const std::string& addressStr)
{
    auto portDelimiterIt = std::find(addressStr.cbegin(), addressStr.cend(), ':');
    if (portDelimiterIt == addressStr.cend())
        return addressStr;

    return std::string_view(addressStr.cbegin(), portDelimiterIt);
}

std::optional<uint16_t> getPort(const std::string& addressStr)
{
    auto portDelimiterIt = std::find(addressStr.cbegin(), addressStr.cend(), ':');
    if (portDelimiterIt == addressStr.cend())
        return {};

           // Skip the delimiter
    ++portDelimiterIt;

    if (portDelimiterIt == addressStr.cend())
        return {};

    uint16_t port{};
    auto result = std::from_chars(portDelimiterIt.base(), addressStr.cend().base(), port);
    if (result.ec != std::errc{})
        return {};

    return port;
}
