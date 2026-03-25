#include "AddressUtils.hpp"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>

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

std::string getLocalLanAddress()
{
    ifaddrs* ifList = nullptr;
    if (getifaddrs(&ifList) != 0)
    {
        return {};
    }

    std::string result{};
    for (const ifaddrs* ifa = ifList; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET)
        {
            continue;
        }

        if ((ifa->ifa_flags & IFF_LOOPBACK) != 0 || (ifa->ifa_flags & IFF_UP) == 0)
        {
            continue;
        }

        const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(ifa->ifa_addr);
        char buf[INET_ADDRSTRLEN]{};
        if (inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf)) != nullptr)
        {
            result = buf;
            break;
        }
    }

    freeifaddrs(ifList);
    return result;
}
