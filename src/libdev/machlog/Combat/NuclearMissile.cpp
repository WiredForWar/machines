/*
 * N U C L M I S S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/Combat/NuclearMissile.hpp"

#include "world4d/Scene/Domain.hpp"
#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/World/PlanetDomains.hpp"

#include "machlog/Messaging/MessageBroker.hpp"
#include "machlog/Messaging/Network.hpp"
#include "machlog/Combat/NuclearBomb.hpp"
#include "machlog/Messaging/VoiceMailManager.hpp"

#include "machlog/Race.hpp"
#include "machlog/Races.hpp"
#include "machlog/Stats.hpp"
#include "machlog/Actors/Actor.hpp"
// #include "machlog/Actors/Actor.hpp"
// #include "machlog/World/SpacialManipulation.hpp"

#include "sim/Manager.hpp"

#include "world4d/Scene/GarbageCollector.hpp"

MachLogNuclearMissile::MachLogNuclearMissile(
    MachLogRace* pRace,
    MachPhysLinearProjectile* pPhysProjectile,
    MachActor* pOwner,
    const MachPhysWeaponData& weaponData)
    : MachLogLinearProjectile(pRace, pPhysProjectile, pOwner, weaponData, DO_NOT_DETECT_INTERSECTIONS)
    , weaponData_(weaponData)
    , pLogRace_(pRace)
{
    // send voicemail if it wasn't me initiating this launch
    if (MachLogRaces::instance().playerRace() != pOwner->race())
        MachLogVoiceMailManager::instance().postNewMail(
            VID_POD_ENEMY_NUKE_LAUNCH,
            MachLogRaces::instance().playerRace());

    TEST_INVARIANT;
}

MachLogNuclearMissile::~MachLogNuclearMissile()
{
    TEST_INVARIANT;
}

void MachLogNuclearMissile::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogNuclearMissile& t)
{

    o << "MachLogNuclearMissile " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogNuclearMissile " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachLogNuclearMissile::doBeDestroyed()
{

    // network stuff....

    if (MachLogNetwork::instance().isNetworkGame())
    {
        MachLogNetwork::instance().messageBroker().sendCreateSpecialWeaponEffectMessage(
            position(),
            MachPhys::NUCLEAR_MISSILE);
    }

    // and create the physical entity on my own node.

    new MachLogNuclearBomb(pLogRace_, globalDestructionPosition(), weaponData_, pOwner());

    /*
    MachLogNuclearBomb* pPhysNuclearBomb = NULL;
    MachLogNuclearMissile::pNewPhysNuclearBomb( globalDestructionPosition(), &pPhysNuclearBomb );

    ASSERT( pPhysNuclearBomb, "We have a NULL pointer to the pPhysNuclearBomb." );

    PhysRelativeTime relTime = 12; // a figure I've chosen arbitrarily that seems to be about long enough.
    pPhysNuclearBomb->startExplosion( SimManager::instance().currentTime(), MachLogPlanet::instance().surface() );
    W4dGarbageCollector::instance().add( pPhysNuclearBomb, relTime + SimManager::instance().currentTime() );

    checkForDamage( MachLogRaces::instance().stats().nuclearMissileBlastRange(), MachLogLinearProjectile::LINEAR_DAMAGE,
    MachPhys::NUCLEAR_MISSILE );
    */
}

// forced recompile 3/2/99 CPS

/* End NUCLMISS.CPP *************************************************/
