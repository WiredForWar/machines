/*
 * S T G M W E P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogMetalStingWeapon

*/

#ifndef _MACHLOG_STGMWEP_HPP
#define _MACHLOG_STGMWEP_HPP

#include "base/base.hpp"
#include "machlog/wepline.hpp"

class MachPhysLinearWeapon;

class MachLogMetalStingWeapon : public MachLogLinearWeapon
// Canonical form revoked
{
public:
    MachLogMetalStingWeapon(MachLogRace*, MachPhysLinearWeapon*, MachActor* pOwner);
    ~MachLogMetalStingWeapon() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogMetalStingWeapon& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogMetalStingWeapon);
    PER_FRIEND_READ_WRITE(MachLogMetalStingWeapon);

protected:
    MachLogLinearProjectile* createLinearProjectile(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset) override;

private:
    MachLogMetalStingWeapon(const MachLogMetalStingWeapon&);
    MachLogMetalStingWeapon& operator=(const MachLogMetalStingWeapon&);
    bool operator==(const MachLogMetalStingWeapon&);
};

PER_DECLARE_PERSISTENT(MachLogMetalStingWeapon);

#endif

/* End STGMWEP.HPP *************************************************/
