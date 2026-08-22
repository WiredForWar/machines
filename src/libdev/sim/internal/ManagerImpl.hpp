#ifndef _SIM_MANAGER_INTERNAL
#define _SIM_MANAGER_INTERNAL

#include "ctl/Vector.hpp"

#include "device/Timer.hpp"

#include "sim/EventDiary.hpp"
#include "sim/sim.hpp"

class SimConditionsManager;
class SimProcess;

class SimManagerImpl
{
public:
    SimManagerImpl();
    ~SimManagerImpl();
    // Data members
    using SimProcesses = ctl_vector<SimProcess*>;
    SimProcesses processes_; // Collection of all SimProcesses
    size_t nextUpdateProcess_; // Index into processes_ of next one to update;
    PhysAbsoluteTime currentTime_; // Current simulation time
    PhysAbsoluteTime lastTimerTime_{}; // Timer reading taken when the current time was last updated
    MATHEX_SCALAR speed_{}; // Multiplier applied to elapsed real time
    PhysAbsoluteTime lastRenderStartTime_; // Time at which last render pass started
    SimEventDiary diary_; // Diary for storing discrete events
    bool suspended_; // True iff simulation activities suspended and time
                     // not advancing
    DevTimer& timer_; // Tracks simulation time
    bool processesAllDeleted_;

    SimActors updateEveryCycleActors_;
    SimPriority totalAmountOfPriority_;
    SimConditionsManager* pConditionsManager_;
    SimActors deadActors_; // Actors waiting to be deleted
};

#endif //_SIM_MANAGER_INTERNAL
