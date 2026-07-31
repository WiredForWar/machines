/*
 * L O C A T O R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include <iostream>

#include "ctl/Algorithm.hpp"
#include "ctl/List.hpp"

#include "mathex/Transform3d.hpp"
#include "mathex/Point2d.hpp"
#include "world4d/Scene/Domain.hpp"

#include "machphys/Machines/GeoLocator.hpp"
#include "machphys/Machines/GeoLocatorData.hpp"

#include "machlog/Actors/GeoLocator.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"

#include "machlog/Messaging/Network.hpp"
#include "machlog/Messaging/MessageBroker.hpp"

PER_DEFINE_PERSISTENT(MachLogGeoLocator);
/* //////////////////////////////////////////////////////////////// */

MachLogGeoLocator::MachLogGeoLocator(
    MachLogMachine::Level hwLevel,
    MachLogMachine::Level swLevel,
    MachLogRace* pRace,
    const MexPoint3d& location,
    std::optional<UtlId> withId)
    : MachLogMachine(
        MachPhys::GEO_LOCATOR,
        hwLevel,
        swLevel,
        pRace,
        pNewPhysGeoLocator(hwLevel, swLevel, pRace, location),
        MachLog::GEO_LOCATOR,
        withId)
{
    objectType(MachLog::GEO_LOCATOR);
    hp(data().hitPoints());
    armour(data().armour());
    MachLogRaces::instance().geoLocators(pRace->race()).push_back(this);
}

MachLogGeoLocator::~MachLogGeoLocator()
{
    /* Intentionally Empty  */
}

PhysRelativeTime MachLogGeoLocator::locate()
{
    return 1.0;
}

/* //////////////////////////////////////////////////////////////// */

void MachLogGeoLocator::doOutputOperator(std::ostream& o) const
{
    MachLogMachine::doOutputOperator(o);
}

/* //////////////////////////////////////////////////////////////// */
// static
MachPhysGeoLocator*
MachLogGeoLocator::pNewPhysGeoLocator(Level hwLevel, Level swLevel, MachLogRace* pRace, const MexPoint3d& location)
{
    // get domain and transform to use
    MexTransform3d localTransform;
    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(location, 0, &localTransform);

    // Construct the physical machine
    return new MachPhysGeoLocator(pDomain, localTransform, hwLevel, swLevel, pRace->race());
}
/* //////////////////////////////////////////////////////////////// */

MachPhysGeoLocator& MachLogGeoLocator::physGeoLocator()
{
    return static_cast<MachPhysGeoLocator&>(physObject());
}
//////////////////////////////////////////////////////////////////////////////////////////

const MachPhysGeoLocator& MachLogGeoLocator::physGeoLocator() const
{
    return static_cast<const MachPhysGeoLocator&>(physObject());
}
//////////////////////////////////////////////////////////////////////////////////////////

void MachLogGeoLocator::isLocating(bool doLocate)
{
    if (doLocate != isLocating())
    {
        MachLogNetwork& network = MachLogNetwork::instance();
        if (network.isNetworkGame() && network.remoteStatus(race()) == MachLogNetwork::LOCAL_PROCESS)
            network.messageBroker().sendPlayNormalObjectAnimationMessage(id(), doLocate);
        physGeoLocator().isLocating(doLocate);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

bool MachLogGeoLocator::isLocating() const
{
    return physGeoLocator().isLocating();
}
// virtual
const MachPhysMachineData& MachLogGeoLocator::machineData() const
{
    return data();
}

const MachPhysGeoLocatorData& MachLogGeoLocator::data() const
{
    // return  static_cast< MachPhysGeoLocatorData&>(physMachine().machineData() );
    return static_cast<const MachPhysGeoLocatorData&>(physMachine().machineData());
}

void perWrite(PerOstream& ostr, const MachLogGeoLocator& actor)
{
    const MachLogMachine& base1 = actor;

    ostr << base1;

    MexTransform3d trans = actor.globalTransform();

    // If we don't do this, the large coords of the transform cause are used when read back in to find
    // a planet domain, which of course falls over big time.
    if (actor.insideAPC())
        trans.position(MexPoint3d(10, 10, 0));

    PER_WRITE_RAW_OBJECT(ostr, trans);
    const W4dId id = actor.id();
    PER_WRITE_RAW_OBJECT(ostr, id);
    actor.persistenceWriteStrategy(ostr);
}

void perRead(PerIstream& istr, MachLogGeoLocator& actor)
{
    MachLogMachine& base1 = actor;

    istr >> base1;

    MexTransform3d trans;
    W4dId id;
    PER_READ_RAW_OBJECT(istr, trans);
    PER_READ_RAW_OBJECT(istr, id);

    MachPhysGeoLocator* pPhysGeoLocator
        = MachLogGeoLocator::pNewPhysGeoLocator(base1.hwLevel(), base1.swLevel(), &actor.logRace(), trans.position());
    actor.setObjectPtr(pPhysGeoLocator, trans);
    actor.id(id);
    actor.machineCreated();
    actor.persistenceReadStrategy(istr);
}

MachLogGeoLocator::MachLogGeoLocator(PerConstructor con)
    : MachLogMachine(con)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
/* End LOCATOR.CPP **************************************************/
