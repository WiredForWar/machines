/*
 * P U L S E W E P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogPlasmaWeapon

        Rifle and cannon
*/

#ifndef _MACHLOG_PLASMAWEP_HPP
#define _MACHLOG_PLASMAWEP_HPP

#include "base/base.hpp"
#include "machlog/wepline.hpp"

class MachPhysLinearWeapon;

class MachLogPlasmaWeapon : public MachLogLinearWeapon
// Canonical form revoked
{
public:
    MachLogPlasmaWeapon(MachLogRace*, MachPhysLinearWeapon*, MachActor* pOwner);
    ~MachLogPlasmaWeapon() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogPlasmaWeapon& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogPlasmaWeapon);
    PER_FRIEND_READ_WRITE(MachLogPlasmaWeapon);

protected:
    MachLogLinearProjectile* createLinearProjectile(
        const PhysAbsoluteTime& burstStartTime,
        uint index,
        W4dEntity* pParent,
        const W4dEntity& target,
        const MexPoint3d& targetOffset) override;

private:
    MachLogPlasmaWeapon(const MachLogPlasmaWeapon&);
    MachLogPlasmaWeapon& operator=(const MachLogPlasmaWeapon&);
    bool operator==(const MachLogPlasmaWeapon&);
};

PER_DECLARE_PERSISTENT(MachLogPlasmaWeapon);

#endif

/* End PULSEWEP.HPP *************************************************/
