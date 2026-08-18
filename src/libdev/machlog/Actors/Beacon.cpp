/*
 * BEACON . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "ctl/List.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/AlignedBox2d.hpp"
#include "mathex/EulerAngles.hpp"
#include "phys/phys.hpp"
#include "world4d/Scene/Domain.hpp"
#include "machphys/Constructions/Beacon.hpp"
#include "machphys/Constructions/BeaconData.hpp"
#include "machphys/Data/Data.hpp"

#include "machlog/World/MachLog.hpp"
#include "machlog/Actors/Beacon.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"

#include "machlog/Messaging/MessageBroker.hpp"

#ifndef _INLINE
#include "machlog/Actors/Beacon.ipp"
#endif

PER_DEFINE_PERSISTENT(MachLogBeacon);
//////////////////////////////////////////////////////////////////////////////////////////

MachLogBeacon::MachLogBeacon(
    MachLogRace* pRace,
    uint level,
    const MexPoint3d& location,
    const MexRadians& angle,
    std::optional<UtlId> withId)
    : MachLogConstruction(
        pRace,
        pNewPhysBeacon(pRace, level, location, angle),
        MachLog::BEACON,
        MachPhysData::instance().beaconData(level),
        withId)
{

    TEST_INVARIANT;
    armour(data().armour());
    MachLogRaces::instance().beacons(pRace->race()).push_back(this);
}
//////////////////////////////////////////////////////////////////////////////////////////

MachLogBeacon::~MachLogBeacon()
{
    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

void MachLogBeacon::CLASS_INVARIANT
{
    INVARIANT(this);
}
//////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachLogBeacon& t)
{

    o << "MachLogBeacon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogBeacon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}
//////////////////////////////////////////////////////////////////////////////////////////
// static
MachPhysBeacon*
MachLogBeacon::pNewPhysBeacon(MachLogRace* pRace, uint level, const MexPoint3d& location, const MexRadians& zAngle)
{
    // get domain and transform to use
    MexTransform3d localTransform;
    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(location, zAngle, &localTransform);

    // Construct the smelter
    return new MachPhysBeacon(pDomain, localTransform, level, pRace->race());
}
//////////////////////////////////////////////////////////////////////////////////////////

// virtual
PhysRelativeTime MachLogBeacon::update(const PhysRelativeTime&, MATHEX_SCALAR)
{
    bool complete = isComplete();

    PhysRelativeTime interval = 5.0;

    if (! updateCompletionVisualisation())
        interval = 0.1;
    else
    {
        if (complete)
            if (! pPhysBeacon()->isWorking())
            {
                pPhysBeacon()->isWorking(true);
                MachLogMessageBroker& broker = MachLogMessageBroker::instance();
                if (broker.isPublishing())
                    broker.sendPlayNormalObjectAnimationMessage(id(), true);
            }
    }

    PhysRelativeTime alteredMaxCPUTime = 0.1;
    MATHEX_SCALAR junk = 0;
    MachActor::update(alteredMaxCPUTime, junk);

    return interval;
}
//////////////////////////////////////////////////////////////////////////////////////////

MachPhysBeacon* MachLogBeacon::pPhysBeacon()
{
    return (MachPhysBeacon*)(&physObject());
}

const MachPhysBeacon* MachLogBeacon::pPhysBeacon() const
{
    return (MachPhysBeacon*)(&physObject());
}
//////////////////////////////////////////////////////////////////////////////////////////

// virtual
const MachPhysConstructionData& MachLogBeacon::constructionData() const
{
    return data();
}

const MachPhysBeaconData& MachLogBeacon::data() const
{
    return pPhysBeacon()->data();
}

void perWrite(PerOstream& ostr, const MachLogBeacon& actor)
{
    const MachLogConstruction& base1 = actor;

    ostr << base1;

    const MexTransform3d& trans = actor.globalTransform();
    PER_WRITE_RAW_OBJECT(ostr, trans);
    const W4dId id = actor.id();
    PER_WRITE_RAW_OBJECT(ostr, id);
    uint level = actor.level();
    PER_WRITE_RAW_OBJECT(ostr, level);
    bool isWorking = actor.pPhysBeacon()->isWorking();
    PER_WRITE_RAW_OBJECT(ostr, isWorking);

    actor.persistenceWriteStrategy(ostr);
}

void perRead(PerIstream& istr, MachLogBeacon& actor)
{
    MachLogConstruction& base1 = actor;

    istr >> base1;

    MexTransform3d trans;
    W4dId id;
    uint level;
    PER_READ_RAW_OBJECT(istr, trans);
    PER_READ_RAW_OBJECT(istr, id);
    PER_READ_RAW_OBJECT(istr, level);
    bool isWorking;
    PER_READ_RAW_OBJECT(istr, isWorking);

    MachPhysBeacon* pPhysBeacon = MachLogBeacon::pNewPhysBeacon(
        &actor.logRace(),
        level,
        trans.position(),
        trans.rotationAsEulerAngles().azimuth());
    actor.setObjectPtr(pPhysBeacon, trans);
    actor.id(id);
    actor.constructionCreated(pPhysBeacon);
    MachLogRaces::instance().nConstructions(actor.race())++;
    pPhysBeacon->isWorking(isWorking);
    actor.persistenceReadStrategy(istr);
}

MachLogBeacon::MachLogBeacon(PerConstructor con)
    : MachLogConstruction(con)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
/* End BEACON.CPP **************************************************/
