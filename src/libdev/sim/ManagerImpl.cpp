#include "sim/internal/ManagerImpl.hpp"

#include "device/Timer.hpp"

#include "sim/Process.hpp"
#include "sim/ConditionsManager.hpp"

SimManagerImpl::SimManagerImpl()
    : suspended_(false)
    , nextUpdateProcess_(0)
    , lastRenderStartTime_(0)
    , timer_(*new DevTimer)
    , processesAllDeleted_(true)
    , totalAmountOfPriority_(0)
{
    updateEveryCycleActors_.reserve(24);
    deadActors_.reserve(24);
}

SimManagerImpl::~SimManagerImpl()
{
    // Empty
}
