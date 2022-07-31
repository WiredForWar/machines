#ifndef MACHLOG_RecentEventsManager_HPP
#define MACHLOG_RecentEventsManager_HPP

#include "mathex/point2d.hpp"
#include "machlog/vmdata.hpp"

#include <vector>

class GuiKeyEvent;
class MachActor;
class MachCameras;

enum class MachineVoiceMailEventID;

class MachLogRecentEventsManager
{
public:
	static MachLogRecentEventsManager& instance();

	void setCameras( MachCameras* pCameras );

	void clear();

	bool hasEvents() const;

	void onVoiceMailPosted(const MexPoint3d& position, VoiceMailID id);
	void onVoiceMailPosted(const MachActor& fromActor, MachineVoiceMailEventID id);

	bool doHandleKeyEvent(const GuiKeyEvent& event);

private:
	MachLogRecentEventsManager();

	void addEventPosition(const MexPoint2d& newEventPos);

	int skipEvents_ = 0;
	std::vector<MexPoint2d> eventPositions_;
	MachCameras *pCameras_ = nullptr;
};

#endif // MACHLOG_RecentEventsManager_HPP
