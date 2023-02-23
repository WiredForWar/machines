/*
 * N U C L B O M B . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogNuclearBomb

    Triggers the physical ion beam animation and handles the logical explosion.
*/

#ifndef _MACHLOG_NUCLBOMB_HPP
#define _MACHLOG_NUCLBOMB_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "machlog/expandbl.hpp"
#include "world4d/world4d.hpp"

// forward declarations
class MachPhysNuclearBomb;
class MachLogNuclearBombImpl;
class MachPhysWeaponData;
class MachLogRace;
class MachActor;

class MachLogNuclearBomb : public MachLogExpandingBlast
// Canonical form revoked
{
public:
    MachLogNuclearBomb(
        MachLogRace* pRace,
        const MexPoint3d& startPosition,
        const MachPhysWeaponData&,
        MachActor* pOwner);

    ~MachLogNuclearBomb() override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachLogNuclearBomb& t);

    static MachPhysNuclearBomb*
    pNewPhysNuclearBomb(const MexPoint3d& startPosition, MachPhysNuclearBomb** ppPhysNuclearBomb);

protected:
    PhysAbsoluteTime firstWaveStartTime() const override;
    PhysAbsoluteTime firstWaveFinishTime() const override;
    PhysAbsoluteTime secondWaveStartTime() const override;
    PhysAbsoluteTime secondWaveFinishTime() const override;
    PhysAbsoluteTime thirdWaveStartTime() const override;
    PhysAbsoluteTime thirdWaveFinishTime() const override;

    bool hitVictimFirstWave(const MachActor& victim) const override;
    void inflictDamageFirstWave(MachActor* pVictim) override;
    bool hitVictimSecondWave(const MachActor& victim) const override;
    void inflictDamageSecondWave(MachActor* pVictim) override;
    bool hitVictimThirdWave(const MachActor& victim) const override;
    void inflictDamageThirdWave(MachActor* pVictim) override;

    PhysAbsoluteTime destructionTime() const override;

    MATHEX_SCALAR potentialKillRadiusMultiplier() const override;

private:
    // opeartions deliberately revoked
    MachLogNuclearBomb(const MachLogNuclearBomb&);
    MachLogNuclearBomb& operator=(const MachLogNuclearBomb&);
    bool operator==(const MachLogNuclearBomb&);

    MachLogNuclearBombImpl* pImpl();
    MachPhysNuclearBomb* pNewPhysNuclearBomb(const MexPoint3d& startPosition, const MachPhysWeaponData& weaponData);

    void doBeDestroyed();

    // data members
    MachLogNuclearBombImpl* pImpl_;
};

#endif

/* End NUCLBOMB.HPP *************************************************/
