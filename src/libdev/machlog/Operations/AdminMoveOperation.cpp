/*
 * O P A D L O C . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */
#include "machlog/Operations/AdminMoveOperation.hpp"

#include "machlog/Internal/AdminMoveOperationImpl.hpp"

#include "ctl/PtrVector.hpp"

#include "mathex/AlignedBox2d.hpp"
#include "mathex/Transform3d.hpp"

#include "phys/Plans/MotionChunk.hpp"

#include "machphys/Machines/MachineMoveInfo.hpp"
#include "machphys/Machines/MachineData.hpp"
#include "machphys/Random.hpp"

#include "machlog/Operations/MoveOperation.hpp"
#include "machlog/Actors/Administrator.hpp"
#include "machlog/World/MineralSite.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Operations/FollowOperation.hpp"
#include "machlog/Operations/ConvoyOffsets.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/Actors/Squadron.hpp"
#include "machlog/Operations/Strategy.hpp"
#include "machlog/Actors/Construction.hpp"
#include "machlog/Operations/GroupMoveInfo.hpp"
#include "machlog/Operations/GroupSimpleMove.hpp"
#include "machlog/Races.hpp"

/* //////////////////////////////////////////////////////////////// */
PER_DEFINE_PERSISTENT(MachLogAdminMoveToOperation);

#define CB_MachLogAdminMoveToOperation_DEPIMPL()                                                                       \
    PRE(pImpl_);                                                                                                       \
    CB_DEPIMPL(MachLogAdministrator*, pActor_);                                                                        \
    CB_DEPIMPL(bool, complete_);                                                                                       \
    CB_DEPIMPL(MexPoint3d, dest_);                                                                                     \
                                                                                                                       \
    CB_DEPIMPL(bool, checkForLeave_);                                                                                  \
    CB_DEPIMPL(MachLogAdminMoveToOperation::Path, path_);                                                              \
    CB_DEPIMPL(MachLogFollowOperation::CamouflagedOp, camoStatus_);

MachLogAdminMoveToOperation::MachLogAdminMoveToOperation(
    MachLogAdministrator* pActor,
    const MexPoint3d& dest,
    bool checkForLeave,
    MachLogFollowOperation::CamouflagedOp camoStatus)
    : MachLogOperation("MOVE_TO_OPERATION", MachLogOperation::MOVE_TO_OPERATION)
    , pImpl_(new MachLogAdminMoveToOperationImpl(pActor, dest, checkForLeave, camoStatus))
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    path_.push_back(dest_);
}

MachLogAdminMoveToOperation::MachLogAdminMoveToOperation(
    MachLogAdministrator* pActor,
    const Path& path,
    PhysPathFindingPriority priority,
    bool checkForLeave,
    MachLogFollowOperation::CamouflagedOp camoStatus)

    : MachLogOperation("MOVE_TO_OPERATION", MachLogOperation::MOVE_TO_OPERATION, priority)
    , pImpl_(new MachLogAdminMoveToOperationImpl(pActor, path, checkForLeave, camoStatus))
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    dest_ = path_.front();
}

// virtual
MachLogAdminMoveToOperation::~MachLogAdminMoveToOperation()
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    if (camoStatus_ == MachLogFollowOperation::CAMOUFLAGED)
        pActor_->camouflaged(false);

    while (path_.size() > 0)
        path_.erase(path_.begin());

    delete pImpl_;
}

void MachLogAdminMoveToOperation::doOutputOperator(std::ostream& o) const
{
    o << "MachLogAdminMoveToOperation" << std::endl;
}

///////////////////////////////////

bool MachLogAdminMoveToOperation::doStart()
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    if (checkForLeave_)
    {
        MachLogConstruction* pConstruction;
        bool insideABuilding = checkNeedLeaveOperation(pActor_, &pConstruction);
        if (insideABuilding && ! pConstruction->globalBoundary().contains(dest_))
            return ! checkNeedAndDoLeaveOperation(pActor_);
    }
    MexPoint2d dest(dest_);
    if (groupMoveInfo().valid())
        pActor_->motionSeq().destination(dest, groupMoveInfo());
    else
        pActor_->motionSeq().destination(dest);

    pActor_->motionSeq().pathFindingPriority(pathFindingPriority());

    return true;
}

PhysRelativeTime MachLogAdminMoveToOperation::doUpdate()
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    HAL_STREAM("MLAdminMoveToOperation::doUpdate\n");
    PhysRelativeTime interval = 5.0;

    int index = 0;
    MachLogGroupSimpleMove::Points points;
    for (MachLogAdminMoveToOperation::Path::iterator i = path_.begin(); i != path_.end(); ++i)
        points.push_back(*i);
    std::string reason;
    ctl_pvector<MachActor> actors;
    for (MachLogSquadron::Machines::iterator i = pActor_->squadron()->machines().begin();
         i != pActor_->squadron()->machines().end();
         ++i)
    {
        actors.push_back(*i);
    }

    MachLogGroupSimpleMove
        groupMove(actors, points, MachLogRaces::instance().AICommandId(), &reason, pathFindingPriority());

    return 5;
}

void MachLogAdminMoveToOperation::doFinish()
{
}

bool MachLogAdminMoveToOperation::doIsFinished() const
{
    CB_MachLogAdminMoveToOperation_DEPIMPL();

    return ! pActor_->motionSeq().hasDestination();
}

bool MachLogAdminMoveToOperation::doBeInterrupted()
{
    return true;
}

void perWrite(PerOstream& ostr, const MachLogAdminMoveToOperation& op)
{
    const MachLogOperation& base1 = op;

    ostr << base1;
    ostr << op.pImpl_;
}

void perRead(PerIstream& istr, MachLogAdminMoveToOperation& op)
{
    MachLogOperation& base1 = op;

    istr >> base1;
    istr >> op.pImpl_;
}

MachLogAdminMoveToOperation::MachLogAdminMoveToOperation(PerConstructor con)
    : MachLogOperation(con)
{
}

/* //////////////////////////////////////////////////////////////// */
