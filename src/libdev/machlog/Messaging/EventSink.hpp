#pragma once

struct MachLogNetMessage;

// A destination for the events the game publishes as they happen.
class MachLogEventSink
{
public:
    virtual ~MachLogEventSink() = default;

    // Takes one published event. The message is guaranteed to live only for the
    // duration of the call, so anything that must outlive it has to be copied.
    virtual void consume(const MachLogNetMessage&) = 0;
};
