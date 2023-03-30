/*
 * V O R T B O M B . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogPunchBlast

    A blasting punch, involving punching.
*/

#ifndef _MACHLOG_PUNCH_HPP
#define _MACHLOG_PUNCH_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "machlog/expandbl.hpp"
#include "world4d/world4d.hpp"

// forward declarations
class MachPhysPunchBlast;
class MachLogPunchBlastImpl;
class MachPhysWeaponData;
class MachLogRace;
class MachActor;

class MachLogPunchBlast : public MachLogExpandingBlast
// Canonical form revoked
{
public:
    MachLogPunchBlast(
        MachLogRace* pRace,
        MachPhysPunchBlast* pPhysPunchBlast,
        const MachPhysWeaponData&,
        MachActor* pOwner);

    ~MachLogPunchBlast() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogPunchBlast& t);

protected:
    PhysAbsoluteTime firstWaveStartTime() const override;
    PhysAbsoluteTime firstWaveFinishTime() const override;

    bool hitVictimFirstWave(const MachActor& victim) const override;
    void inflictDamageFirstWave(MachActor* pVictim) override;

    PhysAbsoluteTime destructionTime() const override;

    MATHEX_SCALAR potentialKillRadiusMultiplier() const override;

    void ownerDestroyed() override;

private:
    // opeartions deliberately revoked
    MachLogPunchBlast(const MachLogPunchBlast&);
    MachLogPunchBlast& operator=(const MachLogPunchBlast&);
    bool operator==(const MachLogPunchBlast&);

    void doBeDestroyed();

    // data members
    MachLogPunchBlastImpl* pImpl_;
};

#endif

/* End PUNCH.HPP *************************************************/
