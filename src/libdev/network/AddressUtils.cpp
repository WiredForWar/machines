#include "AddressUtils.hpp"

#include <algorithm>
#include <charconv>
#include <string>

std::string_view getHost(const std::string_view& addressStr)
{
    auto portDelimiterIt = std::find(addressStr.cbegin(), addressStr.cend(), ':');
    if (portDelimiterIt == addressStr.cend())
        return addressStr;

    return std::string_view(addressStr.cbegin(), portDelimiterIt);
}

std::optional<uint16_t> getPort(const std::string_view& addressStr)
{
    auto portDelimiterIt = std::find(addressStr.cbegin(), addressStr.cend(), ':');
    if (portDelimiterIt == addressStr.cend())
        return {};

    // Skip the delimiter
    ++portDelimiterIt;

    if (portDelimiterIt == addressStr.cend())
        return {};

    uint16_t port{};
    auto result = std::from_chars(portDelimiterIt, addressStr.cend(), port);
    if (result.ec != std::errc{})
        return {};

    return port;
}

std::string makeAddress(std::string_view ip, uint16_t port)
{
    return std::string(ip) + ":" + std::to_string(port);
}
