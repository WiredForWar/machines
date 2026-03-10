/*
 * O P A D D E C O . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved.
 */

#include "ctl/PtrVector.hpp"

#include "mathex/Point2d.hpp"

#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "phys/Plans/MotionChunk.hpp"

#include "machphys/Machines/MachineData.hpp"
#include "machphys/Machines/MachineMoveInfo.hpp"
#include "machphys/Random.hpp"

#include "machlog/Actors/Administrator.hpp"
#include "machlog/Controllers/Controller.hpp"
#include "machlog/Actors/Constructor.hpp"
#include "machlog/Actors/Construction.hpp"
#include "machlog/Operations/FollowOperation.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/Operations/MoveOperation.hpp"
#include "machlog/Operations/AdminDeconstructOperation.hpp"
#include "machlog/Operations/DeconstructOperation.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Races.hpp"
#include "machlog/Actors/ResourceCarrier.hpp"
#include "machlog/Actors/Squadron.hpp"
#include "machlog/Operations/Strategy.hpp"

PER_DEFINE_PERSISTENT(MachLogAdminDeconstructOperation);

MachLogAdminDeconstructOperation::MachLogAdminDeconstructOperation(
    MachLogAdministrator* pActor,
    MachLogConstruction* pConstruction_)
    : MachLogOperation("ADMIN_DECONSTRUCT_OPERATION", MachLogOperation::ADMIN_DECONSTRUCT_OPERATION)
    , pActor_(pActor)
    , complete_(false)
    , pConstruction_(pConstruction_)
    , currentlyAttached_(true)
{
    ASSERT(pConstruction_, "Pointer to construction passed into MLAdminDeconstructOp is NULL\n");
    pConstruction_->attach(this);
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachLogAdminDeconstructOperation::~MachLogAdminDeconstructOperation()
{
    if (currentlyAttached_)
        pConstruction_->detach(this);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachLogAdminDeconstructOperation::doOutputOperator(std::ostream& o) const
{
    o << "MachLogAdminDeconstructOperation\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MachLogAdminDeconstructOperation::doStart()
{
    return ! checkNeedAndDoLeaveOperation(pActor_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PhysRelativeTime MachLogAdminDeconstructOperation::doUpdate()
{
    // HAL_STREAM( "MADConOp::doChooseSubOperation entry for " << pActor_->id() << std::endl);
    ////HAL_STREAM( "  Incomplete mine? " << haveIncompleteMine << std::endl;)

    // See if all our constructors are idle
    bool taskedConstructor = false;
    bool allConstructorsIdle = true;
    ctl_pvector<MachLogMachine> constructors;
    int nConstructors = 0;
    for (MachLogSquadron::Machines::iterator i = pActor_->squadron()->machines().begin();
         i != pActor_->squadron()->machines().end();
         ++i)
    {
        if ((*i)->objectType() == MachLog::CONSTRUCTOR)
        {
            constructors.push_back((*i));
            ++nConstructors;
            if (!(*i)->isIdle())
                allConstructorsIdle = false;
        }
    }

    // can't continue the op with no constructors - why, it would be madness!
    if (nConstructors == 0)
    {
        complete_ = true;

        // If we are the AI race and we have no constructors, dissolve the squadron as it was only intended for
        //  constructor-related ops. Free up the other machines for reassignment elsewhere.
        if (MachLogRaces::instance().controller(pActor_->race()).type() == MachLogController::AI_CONTROLLER)
            pActor_->squadron()->releaseAllMachines();

        return 0.0;
    }

    // okay, we still have our target

    // Task the transporter/constructors as required

    for (MachLogSquadron::Machines::iterator i = pActor_->squadron()->machines().begin();
         i != pActor_->squadron()->machines().end();
         ++i)
    {

        // don't interrupt actors who are currently evading aggressors
        if (!(*i)->evading()
            && ((*i)->isIdle() || (*i)->strategy().currentOperationType() == MachLogOperation::ENTER_OPERATION || (*i)->strategy().currentOperationType() == MachLogOperation::FOLLOW_OPERATION))
        {
            if ((*i)->objectType() == MachLog::RESOURCE_CARRIER)
                MachLogResourceCarrier::assignResourceCarrierTask(&(*i)->asResourceCarrier());
            if ((*i)->objectType() == MachLog::AGGRESSOR || (*i)->objectType() == MachLog::ADMINISTRATOR)
                (*i)->newOperation(std::make_unique<MachLogFollowOperation>(
                    (*i),
                    constructors[MachPhysRandom::randomInt(0, (int)constructors.size())],
                    MexPoint2d(MachPhysRandom::randomDouble(-20, 20), MachPhysRandom::randomDouble(-20, 20))));
            if ((*i)->objectType() == MachLog::CONSTRUCTOR /* and  allConstructorsIdle*/)
            {
                MachLogConstructor* pConstructor = &(*i)->asConstructor();
                pConstructor->newOperation(std::make_unique<MachLogDeconstructOperation>(pConstructor, pConstruction_));
                taskedConstructor = true;
            }
        }
        else
        {
            ////HAL_STREAM("  subordinate " << (*i)->id() << " is not idle " << std::endl;)
        }
    }
    // give the controlling administrator something to do.
    if (!pSubOperation())
        subOperation(
            pActor_,
            std::make_unique<MachLogFollowOperation>(
                pActor_,
                constructors[MachPhysRandom::randomInt(0, (int)constructors.size())],
                MexPoint2d(MachPhysRandom::randomDouble(-20, 20), MachPhysRandom::randomDouble(-20, 20))));
    while (constructors.size())
        constructors.erase(constructors.begin());

    return 15.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachLogAdminDeconstructOperation::doFinish()
{
    /* Intentionally Empty  */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MachLogAdminDeconstructOperation::doIsFinished() const
{
    return complete_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MachLogAdminDeconstructOperation::doBeInterrupted()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
bool MachLogAdminDeconstructOperation::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int)
{
    PRE(pSubject == (W4dSubject*)pConstruction_);

    bool stayAsObserver = true;

    switch (event)
    {
        case W4dSubject::DELETED:
            {
                // construction has been deleted
                pConstruction_ = nullptr;
                currentlyAttached_ = false;
                complete_ = true;
                stayAsObserver = false;
            }
            break;
        default:;
    }
    ////HAL_STREAM(" returning true\n" );
    return stayAsObserver;
}

void perWrite(PerOstream& ostr, const MachLogAdminDeconstructOperation& op)
{
    const MachLogOperation& base1 = op;

    ostr << base1;
    ostr << op.pActor_;
    ostr << op.pConstruction_;
    ostr << op.complete_;
    ostr << op.currentlyAttached_;
}

void perRead(PerIstream& istr, MachLogAdminDeconstructOperation& op)
{
    MachLogOperation& base1 = op;

    istr >> base1;
    istr >> op.pActor_;
    istr >> op.pConstruction_;
    istr >> op.complete_;
    istr >> op.currentlyAttached_;
    if (op.currentlyAttached_)
        op.pConstruction_->attach(&op);
}

MachLogAdminDeconstructOperation::MachLogAdminDeconstructOperation(PerConstructor con)
    : MachLogOperation(con)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* End OPADDECO.CPP **************************************************/
