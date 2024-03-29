/*
 * S T G M W E P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysMetalStingWeapon

    the wast metal sting linear weapon
*/

#ifndef _MACHPHYS_STGMWEP_HPP
#define _MACHPHYS_STGMWEP_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "machphys/wepline.hpp"

class W4dEntity;
class MachPhysMetalSting;
class MachPhysMachine;
class MachPhysConstruction;
class MachPhysArtefact;

class MachPhysMetalStingWeapon : public MachPhysLinearWeapon
// Canonical form revoked
{
public:
    // public constructor
    MachPhysMetalStingWeapon(W4dEntity* pParent, const MexTransform3d& localTransform);

    ~MachPhysMetalStingWeapon() override;

    static const MachPhysMetalStingWeapon& exemplar();

    MachPhysLinearProjectile* createProjectile(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset) override;

    MachPhysMetalSting* createMetalSting(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset);

    PhysRelativeTime fire(const PhysAbsoluteTime& startTime, int numberInBurst) override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT(MachPhysMetalStingWeapon);
    PER_FRIEND_READ_WRITE(MachPhysMetalStingWeapon);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachPhysMetalStingWeapon& t);

    MachPhysMetalStingWeapon(const MachPhysMetalStingWeapon&);
    MachPhysMetalStingWeapon& operator=(const MachPhysMetalStingWeapon&);

    // one tiem constructor
    MachPhysMetalStingWeapon();

    friend class MachPhysWeaponPersistence;

    // the composite file path for given type
    static const char* compositeFilePath();
};

PER_DECLARE_PERSISTENT(MachPhysMetalStingWeapon);

#endif

/* End STGMWEP.HPP **************************************************/
