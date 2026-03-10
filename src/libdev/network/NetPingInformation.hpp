#pragma once

#include "network/NodeUid.hpp"

#include "ctl/Map.hpp"

struct NetPingInformation
{
    double lastTimePingAsked_;
    double lastTimePingSet_;
    double lastPingTime_;
    double lastProperPingTime_;
    friend bool operator==(const NetPingInformation& lhs, const NetPingInformation& rhs)
    {
        return lhs.lastTimePingAsked_ == rhs.lastTimePingAsked_ && lhs.lastTimePingSet_ == rhs.lastTimePingSet_
            && lhs.lastPingTime_ == rhs.lastPingTime_ && lhs.lastProperPingTime_ == rhs.lastProperPingTime_;
    }
    friend bool operator<(const NetPingInformation& lhs, const NetPingInformation& rhs)
    {
        return lhs.lastTimePingAsked_ < rhs.lastTimePingAsked_ && lhs.lastTimePingSet_ < rhs.lastTimePingSet_
            && lhs.lastPingTime_ < rhs.lastPingTime_ && lhs.lastProperPingTime_ < rhs.lastProperPingTime_;
    }
    friend std::ostream& operator<<(std::ostream& o, const NetPingInformation& pi);
};

using NetPing = ctl_map<NetNodeUid, NetPingInformation, std::less<NetNodeUid>>;
