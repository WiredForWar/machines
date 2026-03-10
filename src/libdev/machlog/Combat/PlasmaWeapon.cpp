/*
 * P U L S E W E P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/Transform3d.hpp"
#include "mathex/Point2d.hpp" //Only needed for template instantiation

#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
// #include "machphys/Random.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/Combat/PlasmaWeapon.hpp"
#include "machlog/Combat/PlasmaBolt.hpp"

PER_DEFINE_PERSISTENT(MachLogPlasmaWeapon);

MachLogPlasmaWeapon::MachLogPlasmaWeapon(MachLogRace* pRace, MachPhysLinearWeapon* pPhysWeapon, MachActor* pOwner)
    : MachLogLinearWeapon(pRace, pPhysWeapon, pOwner)
{

    TEST_INVARIANT;
}

MachLogPlasmaWeapon::~MachLogPlasmaWeapon()
{
    TEST_INVARIANT;
}

void MachLogPlasmaWeapon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
MachLogLinearProjectile* MachLogPlasmaWeapon::createLinearProjectile(
    const PhysAbsoluteTime& burstStartTime,
    uint index,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    return new MachLogPlasmaBolt(
        &logRace(),
        createPhysLinearProjectile(burstStartTime, index, pParent, target, targetOffset),
        &owner(),
        physWeapon().weaponData());
}

std::ostream& operator<<(std::ostream& o, const MachLogPlasmaWeapon& t)
{

    o << "MachLogPlasmaWeapon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogPlasmaWeapon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogPlasmaWeapon& weapon)
{
    const MachLogLinearWeapon& base1 = weapon;

    ostr << base1;
}

void perRead(PerIstream& istr, MachLogPlasmaWeapon& weapon)
{
    MachLogLinearWeapon& base1 = weapon;

    istr >> base1;
}

MachLogPlasmaWeapon::MachLogPlasmaWeapon(PerConstructor con)
    : MachLogLinearWeapon(con)
{
}

/* End PULSEWEP.CPP *************************************************/
