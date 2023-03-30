/*
 * S T G L W E P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysLightStingWeapon

    A wast light sting linear weapon
*/

#ifndef _MACHPHYS_STGLWEP_HPP
#define _MACHPHYS_STGLWEP_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "machphys/wepline.hpp"

class W4dEntity;
class MachPhysLightSting;
class MachPhysMachine;
class MachPhysConstruction;
class MachPhysArtefact;

class MachPhysLightStingWeapon : public MachPhysLinearWeapon
// Canonical form revoked
{
public:
    // public constructor
    MachPhysLightStingWeapon(W4dEntity* pParent, const MexTransform3d& localTransform);
    ~MachPhysLightStingWeapon() override;

    static const MachPhysLightStingWeapon& exemplar();

    MachPhysLinearProjectile* createProjectile(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset) override;

    MachPhysLightSting* createLightSting(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset);

    PhysRelativeTime fire(const PhysAbsoluteTime& startTime, int numberInBurst) override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT(MachPhysLightStingWeapon);
    PER_FRIEND_READ_WRITE(MachPhysLightStingWeapon);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachPhysLightStingWeapon& t);

    MachPhysLightStingWeapon(const MachPhysLightStingWeapon&);
    MachPhysLightStingWeapon& operator=(const MachPhysLightStingWeapon&);

    enum
    {
        GLOWING
    };
    void startGlow(const PhysAbsoluteTime& startTime);
    void stopGlow();

    // the composite file path for given type
    static const char* compositeFilePath();

    // one time constructor
    MachPhysLightStingWeapon();

    friend class MachPhysWeaponPersistence;
};

PER_DECLARE_PERSISTENT(MachPhysLightStingWeapon);

#endif

/* End STGLWEP.HPP **************************************************/
