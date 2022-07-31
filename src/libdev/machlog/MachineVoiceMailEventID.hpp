#ifndef MACHLOG_MachineVoiceMailEventID_HPP
#define MACHLOG_MachineVoiceMailEventID_HPP

enum class MachineVoiceMailEventID
{
	SELF_DESTRUCT,
	BUILT,
	DESTROYED,
	NEAR_DEATH,
	CHANGED_RACE,
	VIRUS_INFECTED,
	DAMAGED,
	TASKED,
	MOVING,
	SELECTED,
	TARGET_ENEMY,
	RECYCLE,

	HEAL_TARGET,
	HEALING_COMPLETE,

	LAUNCH_VIRUS,
	VIRUS_LAUNCHED,
	TREACHERY_TARGET,

	BUILDING_COMPLETE,
	AWAITING_NEW_JOB,
	MOVING_TO_NEXT,
	MOVE_TO_SITE,
	BUILDING_CAPTURED,
	BUILDING_DECONSTRUCTED,
	BUILDING_REPAIRED,

	MINERAL_LOCATED,
	SEARCH_COMPLETE,
};

#endif // MACHLOG_MachineVoiceMailEventID_HPP
