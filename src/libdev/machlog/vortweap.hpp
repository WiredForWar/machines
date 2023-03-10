/*
 * V O R T W E A P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVortexWeapon

*/

#ifndef _MACHLOG_VORTWEAP_HPP
#define _MACHLOG_VORTWEAP_HPP

#include "base/base.hpp"
#include "machlog/weapon.hpp"

class MachLogFireData;

class MachLogVortexWeapon : public MachLogWeapon
// Canonical form revoked
{
public:
    MachLogVortexWeapon(MachLogRace*, MachPhysWeapon*, MachActor* pOwner);
    ~MachLogVortexWeapon() override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachLogVortexWeapon& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogVortexWeapon);
    PER_FRIEND_READ_WRITE(MachLogVortexWeapon);

protected:
    void doFire(MachActor*, const MachLogFireData&) override;
    void doFire(const MexPoint3d& position) override;

private:
    MachLogVortexWeapon(const MachLogVortexWeapon&);
    MachLogVortexWeapon& operator=(const MachLogVortexWeapon&);
    bool operator==(const MachLogVortexWeapon&);
};

PER_DECLARE_PERSISTENT(MachLogVortexWeapon);

#endif

/* End PULSEWEP.HPP *************************************************/
