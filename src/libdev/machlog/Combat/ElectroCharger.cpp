/*
 * F L A M E T H R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/transf3d.hpp"
#include "mathex/point2d.hpp" //Only needed for template instantiation

#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
// #include "machphys/Random.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/Combat/ElectroCharger.hpp"
#include "machlog/Combat/Electro.hpp"

// added for gun targetting.
// #include "machlog/Actors/Construction.hpp"
// #include "machlog/Actors/Machine.hpp"
// #include "machlog/Actors/MotionSequencer.hpp"
// #include "machlog/World/Planet.hpp"
// #include "phys/ConfigSpace/ConfigSpace2d.hpp"

PER_DEFINE_PERSISTENT(MachLogElectroCharger);

MachLogElectroCharger::MachLogElectroCharger(MachLogRace* pRace, MachPhysLinearWeapon* pPhysWeapon, MachActor* pOwner)
    : MachLogLinearWeapon(pRace, pPhysWeapon, pOwner)
{

    TEST_INVARIANT;
}

MachLogElectroCharger::~MachLogElectroCharger()
{
    TEST_INVARIANT;
}

void MachLogElectroCharger::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
MachLogLinearProjectile* MachLogElectroCharger::createLinearProjectile(
    const PhysAbsoluteTime& burstStartTime,
    uint index,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    return new MachLogElectro(
        &logRace(),
        createPhysLinearProjectile(burstStartTime, index, pParent, target, targetOffset),
        &owner(),
        physWeapon().weaponData());
}

std::ostream& operator<<(std::ostream& o, const MachLogElectroCharger& t)
{

    o << "MachLogElectroCharger " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogElectroCharger " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogElectroCharger& weapon)
{
    const MachLogLinearWeapon& base1 = weapon;

    ostr << base1;
}

void perRead(PerIstream& istr, MachLogElectroCharger& weapon)
{
    MachLogLinearWeapon& base1 = weapon;

    istr >> base1;
}

MachLogElectroCharger::MachLogElectroCharger(PerConstructor con)
    : MachLogLinearWeapon(con)
{
}

/* End PULSEWEP.CPP *************************************************/
