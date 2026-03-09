/*
 * T R E C H O R B . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "ctl/pvector.hpp"
#include "world4d/Scene/Domain.hpp"
#include "sim/manager.hpp"
#include "machphys/Weapons/TreacheryWeapon.hpp"
#include "machphys/Weapons/TreacheryOrb.hpp"
#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machlog/Combat/TreacheryOrb.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"
#include "machlog/Actors/Actor.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/World/SpacialManipulation.hpp"

PER_DEFINE_PERSISTENT(MachLogTreacheryOrb);

MachLogTreacheryOrb::MachLogTreacheryOrb(
    MachLogRace* pRace,
    MachPhysLinearProjectile* pPhysProjectile,
    MachActor* pOwner,
    const MachPhysWeaponData& weaponData)
    : MachLogLinearProjectile(pRace, pPhysProjectile, pOwner, weaponData)
    , race_(pRace->race())
    , pLogRace_(pRace)
{
    HAL_STREAM("MLTreacheryOrb::CTOR\n");
    TEST_INVARIANT;
}

MachLogTreacheryOrb::~MachLogTreacheryOrb()
{
    TEST_INVARIANT;
}

void MachLogTreacheryOrb::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogTreacheryOrb& t)
{

    o << "MachLogTreacheryOrb " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogTreacheryOrb " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachLogTreacheryOrb::doBeDestroyed()
{
    ctl_pvector<MachActor> actors;
    MachLogSpacialManipulation::genericCheckForIntersections(globalDestructionPosition(), 1, &actors);
    HAL_STREAM("MLTreacheryOrb::doBeDestroyed...num of intersections " << actors.size() << std::endl);
    PhysAbsoluteTime now = SimManager::instance().currentTime();
    for (ctl_pvector<MachActor>::iterator i = actors.begin(); i != actors.end(); ++i)
    {
        if ((*i)->race() != race_)
        {
            HAL_STREAM(" we have a hit id: " << (*i)->id() << std::endl);
            if ((*i)->objectIsMachine())
            {
                MachLogMachine& mlm = (*i)->asMachine();
                const MachLogMachine& cmlm = (*i)->asMachine();
                const MachPhysMachine& mpm = cmlm.physMachine();
                MachPhysMachine& ncmpm = _CONST_CAST(MachPhysMachine&, mpm);
                MachPhys::Race oldRace = mlm.race();
                mlm.assignToDifferentRace(*pLogRace_);
                MachPhysTreacheryWeapon::traitorAnimation(&ncmpm, now, oldRace, race_);
                MachPhysTreacheryOrb& mpto = _STATIC_CAST(MachPhysTreacheryOrb&, physLinearProjectile());
                mpto.suckRaceAt(now, mlm.race());
                if (mlm.objectType() == MachLog::TECHNICIAN)
                    MachLogRaces::instance().techniciansTreacheried(
                        race_,
                        MachLogRaces::instance().techniciansTreacheried(race_) + 1);
                break;
            }
        }
    }
}

void perWrite(PerOstream& ostr, const MachLogTreacheryOrb& actor)
{
    const MachLogLinearProjectile& base1 = actor;

    ostr << base1;
}

void perRead(PerIstream& istr, MachLogTreacheryOrb& actor)
{
    MachLogLinearProjectile& base1 = actor;

    istr >> base1;
}

MachLogTreacheryOrb::MachLogTreacheryOrb(PerConstructor con)
    : MachLogLinearProjectile(con)
{
}

/* End TRECHORB.CPP *************************************************/
