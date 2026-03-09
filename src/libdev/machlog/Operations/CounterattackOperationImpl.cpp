/*
 * O P C O U N T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/Internal/CounterattackOperationImpl.hpp"

#include "sim/manager.hpp"

#include "machphys/Random.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Operations/CounterattackOperation.hpp"
#include "machlog/Operations/Operation.hpp"

#include "mathex/point2d.hpp"

PER_DEFINE_PERSISTENT(MachLogCounterattackOperationImpl);

MachLogCounterattackOperationImpl::MachLogCounterattackOperationImpl(MachLogMachine* pActor, MachActor* pTarget)
    : pActor_(pActor)
    , pTarget_(pTarget)
    , finished_(false)
    , initiatedAttackOp_(false)
    , distanceBeyondWeaponRangeLastTimeIChecked_(0.0) // dummy assignment
{
    PhysAbsoluteTime timeNow = SimManager::instance().currentTime();
    lastTimeTargetWasntEvading_ = timeNow;
    nextTimeINeedToCheckImClosing_ = timeNow + MachLogCounterattackOperation::distanceBeyondWeaponRangeCheckInterval()
        + _STATIC_CAST(MATHEX_SCALAR, MachPhysRandom::randomInt(0, 11));

    TEST_INVARIANT;
}

MachLogCounterattackOperationImpl::~MachLogCounterattackOperationImpl()
{
    TEST_INVARIANT;
}

void MachLogCounterattackOperationImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogCounterattackOperationImpl& t)
{

    o << "MachLogCounterattackOperationImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogCounterattackOperationImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogCounterattackOperationImpl& counterattackOpImpl)
{
    ostr << counterattackOpImpl.pActor_;
    ostr << counterattackOpImpl.pTarget_;
    ostr << counterattackOpImpl.finished_;
    ostr << counterattackOpImpl.initiatedAttackOp_;
    ostr << counterattackOpImpl.pCachedOperation_.get();
    ostr << counterattackOpImpl.lastTimeTargetWasntEvading_;
    ostr << counterattackOpImpl.distanceBeyondWeaponRangeLastTimeIChecked_;
    ostr << counterattackOpImpl.nextTimeINeedToCheckImClosing_;
}

void perRead(PerIstream& istr, MachLogCounterattackOperationImpl& counterattackOpImpl)
{
    istr >> counterattackOpImpl.pActor_;
    istr >> counterattackOpImpl.pTarget_;
    istr >> counterattackOpImpl.finished_;
    istr >> counterattackOpImpl.initiatedAttackOp_;
    MachLogOperation *operation{};
    istr >> operation;
    counterattackOpImpl.pCachedOperation_.reset(operation);
    istr >> counterattackOpImpl.lastTimeTargetWasntEvading_;
    istr >> counterattackOpImpl.distanceBeyondWeaponRangeLastTimeIChecked_;
    istr >> counterattackOpImpl.nextTimeINeedToCheckImClosing_;
}

MachLogCounterattackOperationImpl::MachLogCounterattackOperationImpl(PerConstructor)
{
}

/* End OPCOUNTI.CPP ***************************************************/
