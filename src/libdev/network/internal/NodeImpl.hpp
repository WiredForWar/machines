/*
 * N O D E I . H P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

/*
    NetNodeImpl

    A brief description of the class should go in here
*/

#ifndef _NETWORK_NODEI_HPP
#define _NETWORK_NODEI_HPP

#include "device/Timer.hpp"
#include "network/NetDefs.hpp"
#include "network/SessionUid.hpp"
#include "network/Message.hpp"
#include "network/NodeUid.hpp"
#include "network/Node.hpp"

class NetAppSessionUid;
class NetPriority;
class NetMessageBody;
class NetMessage;
class NetPriority;
class NetINetwork;
class NetCompoundMessage;

class NetNodeImpl
// Canonical form revoked
{
public:
    ~NetNodeImpl();

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const NetNodeImpl& t);
    friend class NetNode;

    NetNodeImpl();
    NetNodeImpl(const NetNodeImpl&);
    NetNodeImpl& operator=(const NetNodeImpl&);

    NetNodeUid* pNodeUid_; // ORDER DEPENDENCY
    NetAppSessionUid* pAppSessionUid_; // ORDER DEPENDENCY

    NetMessageBuffer messageBuffer_;

    bool acceptingPing_;

    const std::string pingString_;

    NetNode::Ping ping_;
    double lastPingAllTime_;
    double lastCompoundTransmitTime_;
    DevTimer timer_;
    NetCompoundMessage* pCompoundMessage_;
    ctl_pvector<NetCompoundMessage> cachedCompoundMessages_;
};

#endif

/* End NODEI.HPP ****************************************************/
