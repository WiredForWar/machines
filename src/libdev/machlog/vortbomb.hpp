/*
 * V O R T B O M B . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVortexBomb

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_VORTBOMB_HPP
#define _MACHLOG_VORTBOMB_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "machlog/expandbl.hpp"
#include "world4d/world4d.hpp"

// forward declarations
class MachPhysVortexBomb;
class MachLogVortexBombImpl;
class MachPhysWeaponData;
class MachLogRace;
class MachActor;

class MachLogVortexBomb : public MachLogExpandingBlast
// Canonical form revoked
{
public:
    MachLogVortexBomb(
        MachLogRace* pRace,
        const MexPoint3d& startPosition,
        const MachPhysWeaponData&,
        MachActor* pOwner);

    ~MachLogVortexBomb() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogVortexBomb& t);

    static MachPhysVortexBomb*
    pNewPhysVortexBomb(const MexPoint3d& startPosition, UtlId firingActorId, MachPhysVortexBomb** ppPhysVortexBomb);

protected:
    bool hitVictimFirstWave(const MachActor& victim) const override;
    void inflictDamageFirstWave(MachActor* pVictim) override;

    PhysAbsoluteTime firstWaveStartTime() const override;
    PhysAbsoluteTime firstWaveFinishTime() const override;

    PhysAbsoluteTime destructionTime() const override;

    MATHEX_SCALAR potentialKillRadiusMultiplier() const override;

private:
    // opeartions deliberately revoked
    MachLogVortexBomb(const MachLogVortexBomb&);
    MachLogVortexBomb& operator=(const MachLogVortexBomb&);
    bool operator==(const MachLogVortexBomb&);

    MachLogVortexBombImpl* pImpl();
    MachPhysVortexBomb*
    pNewPhysVortexBomb(const MexPoint3d& startPosition, const MachPhysWeaponData& weaponData, MachActor* pOwnerActor);

    void doBeDestroyed();

    // data members
    MachLogVortexBombImpl* pImpl_;
};

#endif

/* End VORTBOMB.HPP *************************************************/
