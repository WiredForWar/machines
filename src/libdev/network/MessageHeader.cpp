#include "network/MessageHeader.hpp"
// #include "network/Priority.hpp"

#include <enet/enet.h>

#include <ostream>

/*NetMessageHeader::NetMessageHeader( const NetNodeUid& sender, const NetPriority& priority ):
sender_(sender),
priority_(priority)
{
    length_ = sizeof(NetProcessorUid) + sizeof(NetThreadUid) + sizeof(NetInterProcessUid)
            + sizeof(NetProcessUid) + sender.nodeName().length();
}*/

NetMessageHeader::NetMessageHeader(ENetPeer* pPeer, const NetPriority& priority)
    : sender_(pPeer)
    , priority_(priority)
{
    //  length_ = sizeof(NetProcessorUid) + sizeof(NetThreadUid) + sizeof(NetInterProcessUid)
    //          + sizeof(NetProcessUid) + sender.nodeName().length();
}

NetMessageHeader::NetMessageHeader(const NetMessageHeader& copyMe)
    : sender_(const_cast<ENetPeer*>(copyMe.sender()))
    , priority_(copyMe.priority())
{
}

// const NetNodeUid& NetMessageHeader::sender() const
const ENetPeer* NetMessageHeader::sender() const
{
    return sender_;
}

const NetPriority& NetMessageHeader::priority() const
{
    return priority_;
}

std::ostream& operator<<(std::ostream& o, const NetMessageHeader& t)
{
    o << "sender_   " << t.sender_ << std::endl;
    o << "priority_ " << t.priority_.urgency() << std::endl;
    o << "length_   " << t.length_ << std::endl;
    return o;
}
