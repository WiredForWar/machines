#ifndef MACHLOG_RecentEventsManager_HPP
#define MACHLOG_RecentEventsManager_HPP

#include "mathex/Point2d.hpp"
#include "machlog/Messaging/VoiceMailData.hpp"

#include <cstddef>
#include <vector>

class MachActor;

enum class MachineVoiceMailEventID;

class MachLogRecentEventsManager
{
public:
    static MachLogRecentEventsManager& instance();

    void clear();

    bool hasEvents() const;

    void onVoiceMailPosted(const MexPoint3d& position, VoiceMailID id);
    void onVoiceMailPosted(const MachActor& fromActor, MachineVoiceMailEventID id);

    // The event to look at next, from the most recent one back to the oldest and
    // then round again.
    MexPoint2d nextEventPosition();
    // PRE( hasEvents() );

private:
    MachLogRecentEventsManager();

    void addEventPosition(const MexPoint2d& newEventPos);

    std::size_t skipEvents_ = 0;
    std::vector<MexPoint2d> eventPositions_;
};

#endif // MACHLOG_RecentEventsManager_HPP
