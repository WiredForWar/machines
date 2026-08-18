#pragma once

#include "machlog/Messaging/EventSink.hpp"
#include "machlog/Messaging/MessageBroker.hpp"

#include <deque>

// Sends published events to the other hosts of a network game.
//
// The transport can refuse to take more for a while, so events that cannot go out
// immediately are held here and sent later by sendCachedMessages(). Order is preserved:
// once anything is queued, everything queues behind it.
class MachLogNetworkSink : public MachLogEventSink
{
public:
    void consume(const MachLogNetMessage&) override;

    bool hasCachedMessages() const;

    // Sends as many queued events as the transport will take.
    void sendCachedMessages();

private:
    void transmit(const MachLogNetMessage&);

    std::deque<MachLogNetMessage> cachedMessages_{};
};
