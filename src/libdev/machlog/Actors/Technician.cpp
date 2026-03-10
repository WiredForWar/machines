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

#include "machphys/Machines/Technician.hpp"
#include "machphys/Machines/TechnicianData.hpp"

#include "machlog/Actors/Technician.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"

#include "machlog/Messaging/Network.hpp"
#include "machlog/Messaging/MessageBroker.hpp"

PER_DEFINE_PERSISTENT(MachLogTechnician)

/* //////////////////////////////////////////////////////////////// */

MachLogTechnician::MachLogTechnician(
    const MachPhys::TechnicianSubType& subType,
    MachLogMachine::Level hwLevel,
    MachLogMachine::Level swLevel,
    MachLogRace* pRace,
    const MexPoint3d& location,
    std::optional<UtlId> withId)
    : MachLogMachine(
        MachPhys::TECHNICIAN,
        hwLevel,
        swLevel,
        pRace,
        pNewPhysTechnician(subType, hwLevel, swLevel, pRace, location),
        MachLog::TECHNICIAN,
        withId)
    , subType_(subType)
{
    HAL_STREAM("MLTech::CTOR\n");
    hp(data().hitPoints());
    armour(data().armour());
    ctl_append(&MachLogRaces::instance().technicians(pRace->race()), this);
    MachLogRaces::instance().cascadeUpdateForResearch(pRace->race());
    HAL_STREAM("MLTech::CTOR exit\n");
}

MachLogTechnician::~MachLogTechnician()
{
    /* Intentionally Empty  */
}

PhysRelativeTime MachLogTechnician::research()
{
    MachLogNetwork& network = MachLogNetwork::instance();
    if (network.isNetworkGame() && network.remoteStatus(race()) == MachLogNetwork::LOCAL_PROCESS)
        network.messageBroker().sendPlayNormalObjectAnimationMessage(id(), true);
    return physTechnician().research();
}

/* //////////////////////////////////////////////////////////////// */

void MachLogTechnician::doOutputOperator(std::ostream& o) const
{
    MachLogMachine::doOutputOperator(o);
}

/* //////////////////////////////////////////////////////////////// */
// static
MachPhysTechnician* MachLogTechnician::pNewPhysTechnician(
    const MachPhys::TechnicianSubType& subType,
    Level hwLevel,
    Level swLevel,
    MachLogRace* pRace,
    const MexPoint3d& location)
{
    // get domain and transform to use
    MexTransform3d localTransform;
    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(location, 0, &localTransform);

    // Construct the physical machine
    return new MachPhysTechnician(pDomain, localTransform, subType, hwLevel, swLevel, pRace->race());
}
// virtual
const MachPhysMachineData& MachLogTechnician::machineData() const
{
    return data();
}

const MachPhysTechnicianData& MachLogTechnician::data() const
{
    return _STATIC_CAST(const MachPhysTechnicianData&, physMachine().machineData());
}
/* //////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////// */

MachPhysTechnician& MachLogTechnician::physTechnician()
{
    return _STATIC_CAST(MachPhysTechnician&, physObject());
}

const MachPhysTechnician& MachLogTechnician::physTechnician() const
{
    return _STATIC_CAST(const MachPhysTechnician&, physObject());
}

const MachPhys::TechnicianSubType& MachLogTechnician::subType() const
{
    return subType_;
}

int MachLogTechnician::technologyLevel() const
{
    switch (subType_)
    {
        case MachPhys::LAB_TECH:
            return 1;
        case MachPhys::TECH_BOY:
            return 2;
        case MachPhys::BRAIN_BOX:
            return 3;
        default:
            ASSERT_INFO(subType_);
            ASSERT_BAD_CASE;
    }
    return 0;
}

void perWrite(PerOstream& ostr, const MachLogTechnician& actor)
{
    const MachLogMachine& base1 = actor;

    ostr << base1;
    ostr << actor.subType_;

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

void perRead(PerIstream& istr, MachLogTechnician& actor)
{
    MachLogMachine& base1 = actor;

    istr >> base1;
    istr >> actor.subType_;

    MexTransform3d trans;
    W4dId id;
    PER_READ_RAW_OBJECT(istr, trans);
    PER_READ_RAW_OBJECT(istr, id);

    MachPhysTechnician* pPhysTechnician = MachLogTechnician::pNewPhysTechnician(
        actor.subType_,
        base1.hwLevel(),
        base1.swLevel(),
        &actor.logRace(),
        trans.position());
    actor.setObjectPtr(pPhysTechnician, trans);
    actor.id(id);
    actor.machineCreated();
    actor.persistenceReadStrategy(istr);
}

MachLogTechnician::MachLogTechnician(PerConstructor con)
    : MachLogMachine(con)
{
}

/* End TECHNICI.CPP **************************************************/
