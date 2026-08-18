/*
 * M O B I L E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <stdlib.h>

#include "base/base.hpp"

#include "mathex/mathex.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/EulerAngles.hpp"
#include "mathex/AlignedBox2d.hpp"
#include "mathex/AlignedBox3d.hpp"
#include "mathex/Polygon2d.hpp"
#include "phys/Plans/MotionChunk.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "sim/Manager.hpp"

#include "machphys/Machines/Machine.hpp"
#include "machphys/Machines/MachineData.hpp"

#include "machlog/Actors/APC.hpp"
#include "machlog/Actors/Mobile.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/World/DyingEntityEvent.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Actors/ActorMaker.hpp"

#include "machlog/Messaging/Network.hpp"
#include "machlog/Messaging/MessageBroker.hpp"

/* //////////////////////////////////////////////////////////////// */

PER_DEFINE_PERSISTENT_ABSTRACT(MachLogMobile);

MachLogMobile::MachLogMobile(
    MachLogRace* pRace,
    MachPhysMobile* pPhysMobile,
    MachLog::ObjectType ot,
    MATHEX_SCALAR highClearence,
    MATHEX_SCALAR lowClearence,
    std::optional<UtlId> withId)
    : MachActor(pRace, pPhysMobile, ot, withId)
    , pMotionSeq_(new MachLogMachineMotionSequencer(
          this,
          pPhysMobile,
          MexPoint2d(position().x(), position().y()),
          highClearence,
          lowClearence))
{
    PRE(pRace != nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////

MachLogMobile::~MachLogMobile()
{
    TEST_INVARIANT;

    delete pMotionSeq_;
}

/* //////////////////////////////////////////////////////////////// */

MachLogMachineMotionSequencer& MachLogMobile::motionSeq()
{
    ASSERT(pMotionSeq_ != nullptr, logic_error());
    return *pMotionSeq_;
}

/* //////////////////////////////////////////////////////////////// */

const MachLogMachineMotionSequencer& MachLogMobile::motionSeq() const
{
    ASSERT(pMotionSeq_ != nullptr, logic_error());
    return *pMotionSeq_;
}

/* //////////////////////////////////////////////////////////////// */

void MachLogMobile::doEndExplodingAnimation()
{
}

PhysRelativeTime MachLogMobile::beDestroyed()
{
    // HAL_STREAM(" (" << id() << ") MLMobile::beDestroyed\n" );
    PhysRelativeTime result = physMobile().beDestroyed();

    // Ensure the physical model sticks around for long enough to do the exploding animation
    preservePhysicalModel(result);

    // Add some debris
    dropDebris(SimManager::instance().currentTime());

    return result;
}

/* //////////////////////////////////////////////////////////////// */

MachPhysMobile& MachLogMobile::physMobile()
{
    return static_cast<MachPhysMobile&>(physObject());
}

const MachPhysMobile& MachLogMobile::physMobile() const
{
    return static_cast<const MachPhysMobile&>(physObject());
}

/* //////////////////////////////////////////////////////////////// */

PhysRelativeTime MachLogMobile::turn(const MexRadians& radians)
{
    MachLogMessageBroker& broker = MachLogMessageBroker::instance();
    if (broker.isPublishing())
        broker.sendMachineTurnMessage(physMobile().id(), radians);

    return physMobile().turn(radians);
}

PhysRelativeTime MachLogMobile::turnWithoutEcho(const MexRadians& radians)
{
    return physMobile().turn(radians);
}

/*
PhysRelativeTime MachLogMobile::move( const MexPoint3d& dest )
{
    return physMobile().move( dest );
}
*/
/* //////////////////////////////////////////////////////////////// */

void MachLogMobile::checkLeaderDestination()
{
    // Check we are following someone
    if (pMotionSeq_->isFollowing())
    {
        // Tell the motion sequencer
        pMotionSeq_->leaderChangedDestination();

        // Ensure we get updated soon
        nextUpdateTime(SimManager::instance().currentTime());
    }
}

void MachLogMobile::preservePhysicalModel(const PhysRelativeTime& forTime)
{
    MachLogMachine& mlm = asMachine();

    MachLogDyingEntityEvent::InsideBuilding insideBuilding;
    MachLogConstruction* pConstruction;

    if (mlm.insideBuilding())
    {
        insideBuilding = MachLogDyingEntityEvent::INSIDE_BUILDING;
        pConstruction = &mlm.insideWhichBuilding();
    }
    else
    {
        insideBuilding = MachLogDyingEntityEvent::NOT_INSIDE_BUILDING;
        pConstruction = nullptr;
    }

    MachLogDyingEntityEvent* pEvent
        = new MachLogDyingEntityEvent(physObjectPtr(), nullptr, forTime, insideBuilding, pConstruction);
    SimManager::instance().add(pEvent);
}

void MachLogMobile::dropDebris(const PhysAbsoluteTime&)
{
    if (!MachLogNetwork::instance().isNetworkGame() || isSimulatedHere())
    {
        MexPoint3d debrisPosition;

        if (objectIsMachine() && asMachine().insideAPC())
            debrisPosition = asMachine().APCImInside().position();
        else
            debrisPosition = position();

        MexAlignedBox3d machineBox;

        if (physObject().isComposite())
            machineBox = physObject().asComposite().compositeBoundingVolume();

        else
            machineBox = physObject().boundingVolume();

        const MexPoint2d minCorner(machineBox.minCorner().x(), machineBox.minCorner().y());
        const MexPoint2d maxCorner(machineBox.maxCorner().x(), machineBox.maxCorner().y());

        // flying machines don't drop debris
        if (! mobileIsGlider())
        {
            MachLogActorMaker::newLogDebris(
                race(),
                asMachine().machineData().cost() / 2,
                position(),
                MexAlignedBox2d(minCorner, maxCorner)); // MexAlignedBox2d( debrisPosition, 5 ) );
        }
    }
}

void perWrite(PerOstream& ostr, const MachLogMobile& actor)
{
    const MachActor& base1 = actor;
    const MachLogCanMove& base2 = actor;
    const MachLogCanTurn& base3 = actor;

    ostr << base1;
    ostr << base2;
    ostr << base3;
}

void perRead(PerIstream& istr, MachLogMobile& actor)
{
    MachActor& base1 = actor;
    MachLogCanMove& base2 = actor;
    MachLogCanTurn& base3 = actor;

    istr >> base1;
    istr >> base2;
    istr >> base3;

    actor.pMotionSeq_ = nullptr;
}

MachLogMobile::MachLogMobile(PerConstructor con)
    : MachActor(con)
{
}

void MachLogMobile::createNewMachineMotionSequencer(MachPhysMobile* pPhysMobile)
{
    PRE(! pMotionSeq_);
    HAL_STREAM(
        "(" << id() << ") MachLogMobile::createNewMachMotSeq at " << pPhysMobile->globalTransform() << std::endl);
    pMotionSeq_ = new MachLogMachineMotionSequencer(
        this,
        pPhysMobile,
        MexPoint2d(pPhysMobile->globalTransform().position().x(), pPhysMobile->globalTransform().position().y()),
        asMachine().highClearence(),
        asMachine().lowClearence());
}

bool MachLogMobile::machineMotionSequencerDefined() const
{
    return pMotionSeq_ != nullptr;
}

MachLogMobile::ObstacleFlags MachLogMobile::obstacleFlags() const
{
    ObstacleFlags result = 0;

    //  Relies on the fact that MachPhysMobile is a typedef for MachPhysMachine.
    if (mobileIsGlider())
    {
        result = MachLog::OBSTACLE_WATER | MachLog::OBSTACLE_LOW;
    }
    else if (physMobile().locomotionType() == MachPhys::HOVER)
    {
        result = MachLog::OBSTACLE_WATER;
    }

    return result;
}

bool MachLogMobile::mobileIsGlider() const
{
    return physMobile().locomotionType() == MachPhys::GLIDER;
}

/* End MOBILE.CPP ***************************************************/
