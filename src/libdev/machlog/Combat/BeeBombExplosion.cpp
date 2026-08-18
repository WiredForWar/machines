/*
 * B E E B O M E X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/Combat/BeeBombExplosion.hpp"
#include "machlog/Internal/BeeBombExplosionImpl.hpp"

#include "mathex/Line3d.hpp"

#include "phys/ConfigSpace/ConfigSpace2d.hpp"

#include "world4d/Scene/Domain.hpp"
#include "world4d/Entity/Composite.hpp"

#include "machphys/Effects/BeeBombWave.hpp"
#include "machphys/Effects/BeeBombExplosion.hpp"
#include "machphys/Random.hpp"
#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Data/Data.hpp"

#include "sim/Manager.hpp"

#include "machlog/Actors/Actor.hpp"
#include "machlog/Combat/ExpandingBlast.hpp"
#include "machlog/Messaging/MessageBroker.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"

MachLogBeeBombExplosion::MachLogBeeBombExplosion(
    MachLogRace* pRace,
    const MexPoint3d& startPosition,
    const MachPhysWeaponData& weaponData,
    MachActor* pOwner)
    : MachLogExpandingBlast(
        pRace,
        pNewPhysBeeBombExplosion(startPosition, weaponData), // <- note that physical BombExplosion is created here
        startPosition,
        pOwner,
        40,
        CANT_HIT_AIR_UNITS) // errrrm?
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    ASSERT(pPhysBeeBombExplosion_, "Unexpected NULL for pPhysBeeBombExplosion_!");

    PhysAbsoluteTime timeNow = SimManager::instance().currentTime();

    firstWaveStartTime_ = timeNow;
    firstWaveFinishTime_
        = timeNow + pPhysBeeBombExplosion_->startExplosion(timeNow, *MachLogPlanet::instance().surface()) - 1.3;

    destructionTime_ = firstWaveFinishTime_ + 1.0;

    // Echo explosion effect across network.
    MachLogMessageBroker& broker = MachLogMessageBroker::instance();
    if (broker.isPublishing())
    {
        broker.sendCreateSpecialWeaponEffectMessage(startPosition, MachPhys::BEE_BOMB);
    }

    // set up collison data and animations

    // setUpBuildingCollisions( startPosition );

    TEST_INVARIANT;
}

// virtual
PhysAbsoluteTime MachLogBeeBombExplosion::firstWaveStartTime() const
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    return firstWaveStartTime_;
}

// virtual
PhysAbsoluteTime MachLogBeeBombExplosion::firstWaveFinishTime() const
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    return firstWaveFinishTime_;
}

// virtual
PhysAbsoluteTime MachLogBeeBombExplosion::destructionTime() const
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    return destructionTime_;
}

MachLogBeeBombExplosion::~MachLogBeeBombExplosion()
{
    TEST_INVARIANT;

    delete pImpl_;
}

void MachLogBeeBombExplosion::CLASS_INVARIANT
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogBeeBombExplosion& t)
{

    o << "MachLogBeeBombExplosion " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogBeeBombExplosion " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachLogBeeBombExplosion::doBeDestroyed()
{
    //  MachLogLinearProjectile::genericCheckForDamage( 0.25, MachLogLinearProjectile::CONSTANT_DAMAGE,
    //  MachPhys::PULSE_RIFLE );
    TEST_INVARIANT;
}

MachPhysBeeBombExplosion*
MachLogBeeBombExplosion::pNewPhysBeeBombExplosion(const MexPoint3d& startPosition, const MachPhysWeaponData& weaponData)
{
    pImpl_ = new MachLogBeeBombExplosionImpl(&weaponData);

    CB_MachLogBeeBombExplosion_DEPIMPL();

    // just calls into static method
    MachPhysBeeBombExplosion* pPhysBomb = pNewPhysBeeBombExplosion(startPosition, &pPhysBeeBombExplosion_);

    ASSERT(pPhysBeeBombExplosion_, "Unexpected NULL for pPhysBeeBombExplosion_!");

    return pPhysBomb;
}

// static
MachPhysBeeBombExplosion* MachLogBeeBombExplosion::pNewPhysBeeBombExplosion(
    const MexPoint3d& startPosition,
    MachPhysBeeBombExplosion** ppPhysBeeBombExplosion)
{
    HAL_STREAM("MLBeeBombExplosion::pNewPhysBeeBombExplosion\n");
    // get domain and transform to use
    MexTransform3d localTransform;

    const MexRadians zAngle = 0.0;

    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(startPosition, zAngle, &localTransform);

    // Construct the physical missile

    MachPhysBeeBombExplosion* pPhysBeeBombExplosion = new MachPhysBeeBombExplosion(pDomain, localTransform);
    *ppPhysBeeBombExplosion = pPhysBeeBombExplosion;
    return pPhysBeeBombExplosion;

    //
}

// virtual
bool MachLogBeeBombExplosion::hitVictimFirstWave(const MachActor& victim) const
{
    MATHEX_SCALAR checkRadiusSize
        = MachPhysBeeBombWave::radius(SimManager::instance().currentTime() - firstWaveStartTime());

    bool result = actorWithinRadius(victim, checkRadiusSize);

    return result;
}

// virtual
void MachLogBeeBombExplosion::inflictDamageFirstWave(MachActor* pDamagedVictim)
{
    CB_MachLogBeeBombExplosion_DEPIMPL();

    MachActor* pByActor = pOwner();
    if (pOwner() && pOwner()->isDead())
        pByActor = nullptr;

    // don't want to damage this victim any more after this
    finishedWithVictim(pDamagedVictim);

    int damageInflicted = pWeaponData_->damagePoints();

    pDamagedVictim->beHit(damageInflicted, MachPhys::BOLTER, pByActor); //(??)

    TEST_INVARIANT;
}

// virtual
MATHEX_SCALAR MachLogBeeBombExplosion::potentialKillRadiusMultiplier() const
{
    return 1.15;
}

// forced recompile 3/2/99 CPS

/* End BEEBombExplosion.CPP *************************************************/
