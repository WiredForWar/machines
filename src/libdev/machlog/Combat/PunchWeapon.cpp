/*
 * P U N W E A P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/Transform3d.hpp"
#include "world4d/Scene/GarbageCollector.hpp"

#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Weapons/PunchWeapon.hpp"
#include "machphys/Effects/PunchBlast.hpp"

#include "sim/Manager.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Combat/PunchWeapon.hpp"
#include "machlog/Combat/Punch.hpp"

PER_DEFINE_PERSISTENT(MachLogPunchWeapon);

MachLogPunchWeapon::MachLogPunchWeapon(MachLogRace* pRace, MachPhysPunchWeapon* pPhysPunchWeapon, MachActor* pOwner)
    : MachLogWeapon(pRace, pPhysPunchWeapon, pOwner)
    , pPhysPunchWeapon_(pPhysPunchWeapon)
{
    TEST_INVARIANT;
}

MachLogPunchWeapon::~MachLogPunchWeapon()
{
    TEST_INVARIANT;
}

void MachLogPunchWeapon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachLogPunchWeapon::doFire(const MexPoint3d& /*position*/)
{
    MachPhysPunchBlast* pPhysPunchBlast = pPhysPunchWeapon_->createPunchBlast(
        SimManager::instance().currentTime(),
        *(MachLogPlanet::instance().surface()));

    MachActor* pOwner = &owner();
    pOwner->setBusyPeriod(2.5); // temporary guess....should really get exact length of animation from machphys object

    new MachLogPunchBlast(&logRace(), pPhysPunchBlast, physWeapon().weaponData(), pOwner);
}

// virtual
void MachLogPunchWeapon::doFire(MachActor* pTarget, const MachLogFireData&)
{
    MachLogPunchWeapon::doFire(pTarget->position());
}

std::ostream& operator<<(std::ostream& o, const MachLogPunchWeapon& t)
{

    o << "MachLogPunchWeapon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogPunchWeapon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogPunchWeapon& weapon)
{
    const MachLogWeapon& base1 = weapon;

    ostr << base1;
}

void perRead(PerIstream& istr, MachLogPunchWeapon& weapon)
{
    MachLogWeapon& base1 = weapon;

    istr >> base1;
}

MachLogPunchWeapon::MachLogPunchWeapon(PerConstructor con)
    : MachLogWeapon(con)
{
}

void MachLogPunchWeapon::doEchoPunch()
{
    MachPhysPunchBlast* pPhysPunchBlast = pPhysPunchWeapon_->createPunchBlast(
        SimManager::instance().currentTime(),
        *(MachLogPlanet::instance().surface()));
    W4dGarbageCollector::instance().add(pPhysPunchBlast, pPhysPunchBlast->blastEndTime());
}

void MachLogPunchWeapon::setPhysicalPunchWeapon(MachPhysPunchWeapon* pPunchWeapon)
{
    pPhysPunchWeapon_ = pPunchWeapon;
}
/* End PUNWEAP.CPP *************************************************/
