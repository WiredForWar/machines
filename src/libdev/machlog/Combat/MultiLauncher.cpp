/*
 * L M I S S L C H . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/Transform3d.hpp"

#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Weapons/LargeMissile.hpp"
// #include "machphys/Random.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/Combat/MultiLauncher.hpp"
#include "machlog/Combat/LargeMissile.hpp"

// added for gun targetting.
// #include "machlog/Actors/Construction.hpp"
// #include "machlog/Actors/Machine.hpp"
// #include "machlog/Actors/MotionSequencer.hpp"
// #include "machlog/World/Planet.hpp"
// #include "phys/ConfigSpace/ConfigSpace2d.hpp"

PER_DEFINE_PERSISTENT(MachLogLargeMissileLauncher);

MachLogLargeMissileLauncher::MachLogLargeMissileLauncher(
    MachLogRace* pRace,
    MachPhysLargeMissile* pPhysWeapon,
    MachActor* pOwner)
    : MachLogLinearWeapon(pRace, pPhysWeapon, pOwner)
    , pPhysLargeMissile_(pPhysWeapon)
{

    TEST_INVARIANT;
}

MachLogLargeMissileLauncher::~MachLogLargeMissileLauncher()
{
    TEST_INVARIANT;
}

void MachLogLargeMissileLauncher::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
MachLogLinearProjectile* MachLogLargeMissileLauncher::createLinearProjectile(
    const PhysAbsoluteTime& burstStartTime,
    uint index,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    HAL_STREAM("MLMultiLauncher::createLinearProjectile\n");
    return new MachLogLargeMissile(
        &logRace(),
        // pPhysLargeMissile_->createMissile( burstStartTime, index, pParent, target, targetOffset ),
        pPhysLargeMissile_
            ->createMissile(burstStartTime, index, pParent, const_cast<W4dEntity&>(target), targetOffset),
        //          createPhysLinearProjectile( burstStartTime, index, pParent, target, targetOffset ),
        &owner(),
        physWeapon().weaponData());
}

std::ostream& operator<<(std::ostream& o, const MachLogLargeMissileLauncher& t)
{

    o << "MachLogLargeMissileLauncher " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogLargeMissileLauncher " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogLargeMissileLauncher& weapon)
{
    const MachLogLinearWeapon& base1 = weapon;

    ostr << base1;
}

void perRead(PerIstream& istr, MachLogLargeMissileLauncher& weapon)
{
    MachLogLinearWeapon& base1 = weapon;

    istr >> base1;
}

MachLogLargeMissileLauncher::MachLogLargeMissileLauncher(PerConstructor con)
    : MachLogLinearWeapon(con)
{
}

/* End LMISSLCH.CPP *************************************************/
