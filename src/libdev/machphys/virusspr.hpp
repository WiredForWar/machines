/*
 * V I R U S S P R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysVirusSpreader

    Models a flame thrower.
*/

#ifndef _MACHPHYS_VIRUSSPR_HPP
#define _MACHPHYS_VIRUSSPR_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "machphys/wepline.hpp"

class W4dEntity;
class MachPhysElectro;
class MachPhysMachine;
class MachPhysConstruction;

class MachPhysVirusSpreader : public MachPhysLinearWeapon
// Canonical form revoked
{
public:
    using MachPhysVirus = MachPhysElectro;

    // Constructs appropriate
    MachPhysVirusSpreader(W4dEntity* pParent, const MexTransform3d& localTransform, MachPhys::Mounting mounting);

    // dtor
    ~MachPhysVirusSpreader() override;

    // Return an exemplar flamethrower - ensures the mesh is loaded
    static const MachPhysVirusSpreader& exemplar(MachPhys::Mounting mounting);

    MachPhysLinearProjectile* createProjectile(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset) override;

    PhysRelativeTime fire(const PhysAbsoluteTime& startTime, int numberInBurst) override;

    PhysRelativeTime victimAnimation(
        const PhysAbsoluteTime& startTime,
        const MexLine3d& fromDirection,
        MachPhysMachine* pMachine) const override;

    PhysRelativeTime victimAnimation(
        const PhysAbsoluteTime& startTime,
        const MexLine3d& fromDirection,
        MachPhysConstruction* pConstruction) const override;

    static void virusFromSourceToTarget(
        W4dEntity* pSourceEntity,
        const MexPoint3d& sourceOffset,
        W4dEntity& targetEntity,
        const MexPoint3d& targetOffset);

    // do the lightning effect and head shake effect on s machine victim
    // returns the lightningDuration+smokeDuration or heatShakingDuration whichever is longer
    static PhysRelativeTime
    applyVictimAnimation(const PhysAbsoluteTime& startTime, const MexLine3d& fromDirection, MachPhysMachine* pMachine);

    static PhysRelativeTime applyVictimAnimation(
        const PhysAbsoluteTime& startTime,
        const MexLine3d& fromDirection,
        MachPhysConstruction* pConstruction);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysVirusSpreader& t);

    PER_MEMBER_PERSISTENT(MachPhysVirusSpreader);

private:
    MachPhysVirusSpreader(const MachPhysVirusSpreader&);
    MachPhysVirusSpreader& operator=(const MachPhysVirusSpreader&);
    bool operator==(const MachPhysVirusSpreader&);

    // One-time constructor (per type) used to create the exemplars
    MachPhysVirusSpreader(MachPhys::Mounting mounting);

    MachPhysVirus* createVirus(
        const PhysAbsoluteTime& startTime,
        const PhysRelativeTime& duration,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset);

    // the composite file path for given type
    static const char* compositeFilePath();

    // Do the halo lightning effect on a victim animation to machine/construction (pComposit).
    static void
    virusHaloInfection(const PhysAbsoluteTime& startTime, const PhysRelativeTime& duration, W4dComposite* pComposit);

    friend class MachPhysWeaponPersistence;
};

PER_DECLARE_PERSISTENT(MachPhysVirusSpreader);
PER_READ_WRITE(MachPhysVirusSpreader);

#endif

/* End VIRUSSPR.HPP ****************************************************/
