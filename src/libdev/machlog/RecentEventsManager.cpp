#include "RecentEventsManager.hpp"

#include "gui/event.hpp"
#include "machgui/cameras.hpp"
#include "machlog/MachineVoiceMailEventID.hpp"
#include "machlog/actor.hpp"

MachLogRecentEventsManager::MachLogRecentEventsManager()
{
}

MachLogRecentEventsManager& MachLogRecentEventsManager::instance()
{
    static MachLogRecentEventsManager instance_;
    return instance_;
}

void MachLogRecentEventsManager::setCameras(MachCameras* pCameras)
{
    pCameras_ = pCameras;
}

void MachLogRecentEventsManager::clear()
{
    eventPositions_.clear();
    skipEvents_ = 0;
}

bool MachLogRecentEventsManager::hasEvents() const
{
    return !eventPositions_.empty();
}

void MachLogRecentEventsManager::onVoiceMailPosted(const MexPoint3d& position, VoiceMailID id)
{
    switch (id)
    {
        case VID_POD_BEACON_DESTROYED:
        case VID_POD_UPLINK_DESTROYED:
        case VID_POD_POD_ATTACKED:
        case VID_POD_POD_CRITICAL_DAMAGE:
        case VID_POD_ION_CANNON_ACQUIRED:
        case VID_POD_ION_CANNON_CHARGED:
        case VID_POD_BUILDING_DESTROYED:
        case VID_POD_BUILDING_CRITICAL:
        case VID_POD_CONSTRUCTION_ATTACKED:
        case VID_POD_MINERALS_EXHAUSTED:
        case VID_POD_MINE_INOPERABLE:
        case VID_POD_NUKE_READY:
        case VID_POD_NO_TECHNICIANS:
        case VID_POD_POD_DESTROYED:
        case VID_POD_MINE_SWITCHED_SOURCE:
        case VID_POD_REINFORCEMENTS_ONLINE:
        case VID_POD_TURRET_ATTACKED:
        case VID_POD_INSUFFICIENT_CASH_FOR_NUKE:
        case VID_POD_NEW_CONSTRUCTIONS:
        case VID_POD_INSUFFICIENT_BMUS:
            break;
        default:
            return;
    }

    addEventPosition(position);
}

void MachLogRecentEventsManager::onVoiceMailPosted(const MachActor& fromActor, MachineVoiceMailEventID id)
{
    switch (id)
    {
        case MachineVoiceMailEventID::SELF_DESTRUCT:
        case MachineVoiceMailEventID::TASKED:
        case MachineVoiceMailEventID::MOVING:
        case MachineVoiceMailEventID::SELECTED:
        case MachineVoiceMailEventID::TARGET_ENEMY:
        case MachineVoiceMailEventID::RECYCLE:
        case MachineVoiceMailEventID::HEAL_TARGET:
        case MachineVoiceMailEventID::LAUNCH_VIRUS:
        case MachineVoiceMailEventID::TREACHERY_TARGET:
        case MachineVoiceMailEventID::AWAITING_NEW_JOB:
        case MachineVoiceMailEventID::MOVING_TO_NEXT:
        case MachineVoiceMailEventID::MOVE_TO_SITE:
            return;
        case MachineVoiceMailEventID::BUILT:
        case MachineVoiceMailEventID::DESTROYED:
        case MachineVoiceMailEventID::NEAR_DEATH:
        case MachineVoiceMailEventID::CHANGED_RACE:
        case MachineVoiceMailEventID::VIRUS_INFECTED:
        case MachineVoiceMailEventID::DAMAGED:
        case MachineVoiceMailEventID::HEALING_COMPLETE:
        case MachineVoiceMailEventID::VIRUS_LAUNCHED:
        case MachineVoiceMailEventID::BUILDING_COMPLETE:
        case MachineVoiceMailEventID::BUILDING_CAPTURED:
        case MachineVoiceMailEventID::BUILDING_DECONSTRUCTED:
        case MachineVoiceMailEventID::BUILDING_REPAIRED:
        case MachineVoiceMailEventID::MINERAL_LOCATED:
        case MachineVoiceMailEventID::SEARCH_COMPLETE:
            break;
    }

    addEventPosition(fromActor.position());
}

bool MachLogRecentEventsManager::doHandleKeyEvent(const GuiKeyEvent& event)
{
    PRE(pCameras_);

    if (event.key() == DevKey::SPACE and event.state() == Gui::PRESSED)
    {
        if (hasEvents())
        {
            pCameras_->moveTo(eventPositions_.at(eventPositions_.size() - skipEvents_ - 1));
            skipEvents_++;
            if (skipEvents_ >= eventPositions_.size())
            {
                skipEvents_ = 0;
            }
        }

        return true;
    }

    return false;
}

void MachLogRecentEventsManager::addEventPosition(const MexPoint2d& newEventPos)
{
    constexpr int MaxEvents = 8;
    while (eventPositions_.size() >= MaxEvents)
    {
        eventPositions_.erase(eventPositions_.begin());
    }

    eventPositions_.push_back(newEventPos);
    skipEvents_ = 0;
}
