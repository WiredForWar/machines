#include "machlog/Messaging/NetworkSink.hpp"

#include "base/Diag.hpp"
#include "machlog/Messaging/Network.hpp"
#include "network/Message.hpp"
#include "network/Network.hpp"

void MachLogNetworkSink::consume(const MachLogNetMessage& message)
{
    // The connection can go bad asynchronously. Try to recover, and if that fails drop
    // the event rather than push it into a broken connection.
    if (NetNetwork::instance().currentStatus() != NetNetwork::NETNET_OK)
    {
        MachLogNetwork::instance().update();
        if (NetNetwork::instance().currentStatus() != NetNetwork::NETNET_OK)
            return;
    }

    if (NetNetwork::instance().imStuffed() || hasCachedMessages())
    {
        cachedMessages_.push_back(message);
        DEBUG_STREAM(DIAG_NETWORK, "MachLogNetworkSink::consume cached " << cachedMessages_.size() << std::endl);
    }
    else
    {
        transmit(message);
    }
}

bool MachLogNetworkSink::hasCachedMessages() const
{
    return !cachedMessages_.empty();
}

void MachLogNetworkSink::sendCachedMessages()
{
    while (!cachedMessages_.empty() && !NetNetwork::instance().imStuffed())
    {
        DEBUG_STREAM(
            DIAG_NETWORK,
            "MachLogNetworkSink::sendCachedMessages sending "
                << static_cast<MachLogMessageBroker::MachLogMessageCode>(cachedMessages_.front().header_.messageCode_)
                << ", " << cachedMessages_.size() << " queued" << std::endl);
        transmit(cachedMessages_.front());
        cachedMessages_.pop_front();
    }
}

void MachLogNetworkSink::transmit(const MachLogNetMessage& message)
{
    // Every host in the session gets every event; the transport has no addressing here.
    NetPriority priority(1);
    NetMessageBody body(reinterpret_cast<const unsigned char*>(&message), message.header_.totalLength_);
    NetNetwork::instance().sendMessage(priority, body);
}
