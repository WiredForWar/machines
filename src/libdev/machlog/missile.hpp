/*
 * M I S S I L E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogMissile

*/

#ifndef _MACHLOG_MISSILE_HPP
#define _MACHLOG_MISSILE_HPP

#include "base/base.hpp"
#include "machlog/lineproj.hpp"

class MachPhysMissile;
class MachPhysLinearProjectile;
class MachActor;

class MachLogMissile : public MachLogLinearProjectile
// Canonical form revoked
{
public:
    MachLogMissile(MachLogRace* pRace, MachPhysMissile*, MachActor* pOwner, const MachPhysWeaponData&);

    ~MachLogMissile() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogMissile& t);

    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogMissile);
    PER_FRIEND_READ_WRITE(MachLogMissile);

private:
    void doBeDestroyed() override;

    MachLogMissile(const MachLogMissile&);
    MachLogMissile& operator=(const MachLogMissile&);
    bool operator==(const MachLogMissile&);

    MachPhysMissile* pPhysMissile_;
};

PER_DECLARE_PERSISTENT(MachLogMissile);

#endif

/* End PULSEBLB.HPP *************************************************/
