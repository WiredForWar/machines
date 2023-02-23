/*
 * I O N B E A M . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogIonBeam

    Triggers the physical ion beam animation and handles the logical explosion.
*/

#ifndef _MACHLOG_IONBEAM_HPP
#define _MACHLOG_IONBEAM_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "machlog/expandbl.hpp"
#include "world4d/world4d.hpp"

// forward declarations
class MachPhysIonBeam;
class MachLogIonBeamImpl;
class MachPhysWeaponData;
class MachLogRace;
class MachActor;

class MachLogIonBeam : public MachLogExpandingBlast
// Canonical form revoked
{
public:
    MachLogIonBeam(MachLogRace* pRace, const MexPoint3d& startPosition, const MachPhysWeaponData&, MachActor* pOwner);

    ~MachLogIonBeam() override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachLogIonBeam& t);

    static MachPhysIonBeam* pNewPhysIonBeam(const MexPoint3d& startPosition, MachPhysIonBeam** ppPhysIonBeam);

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
    MachLogIonBeam(const MachLogIonBeam&);
    MachLogIonBeam& operator=(const MachLogIonBeam&);
    bool operator==(const MachLogIonBeam&);

    MachLogIonBeamImpl* pImpl();
    MachPhysIonBeam* pNewPhysIonBeam(const MexPoint3d& startPosition, const MachPhysWeaponData& weaponData);

    void doBeDestroyed();

    // data members
    MachLogIonBeamImpl* pImpl_;
};

#endif

/* End IONBEAM.HPP *************************************************/
