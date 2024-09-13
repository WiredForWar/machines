#ifndef _NETDEFS_HPP
#define _NETDEFS_HPP

// #include <string.hpp>

#include "ctl/utility.hpp"

#include "stdlib/string.hpp"
// #include <std::string.hpp>

using NetProcessorUid = std::string;
using NetAppSessionName = std::string;
using NetNodeName = std::string;
using NetMessageUid = unsigned int;
using NetAppUid = unsigned int;
using NetAppInstanceUid = unsigned int;

using NetInterProcessUid = unsigned int;
using NetThreadUid = unsigned int;

const unsigned char MAXNAMELEN = 200;
const unsigned char OUTWARD = 1;
const unsigned char INWARD = 2;
// special marker to indicate library specific messages - these should NEVER be queued.
const unsigned char NETWORK_LIBRARY_MESSAGE = 0xEF;

CTL_PAIR(NetProcessUid, NetProcessorUid, processorUid, NetThreadUid, threadUid);

#endif //_NETDEFS_HPP
