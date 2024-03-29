/*
 * M I S S I L E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogLargeMissile

*/

#ifndef _MACHLOG_LMISSILE_HPP
#define _MACHLOG_LMISSILE_HPP

#include "base/base.hpp"
#include "machlog/lineproj.hpp"

class MachPhysMissile;
class MachPhysLinearProjectile;
class MachActor;

class MachLogLargeMissile : public MachLogLinearProjectile
// Canonical form revoked
{
public:
    MachLogLargeMissile(MachLogRace* pRace, MachPhysMissile*, MachActor* pOwner, const MachPhysWeaponData&);

    ~MachLogLargeMissile() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogLargeMissile& t);

    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

private:
    void doBeDestroyed() override;

    MachLogLargeMissile(const MachLogLargeMissile&);
    MachLogLargeMissile& operator=(const MachLogLargeMissile&);
    bool operator==(const MachLogLargeMissile&);

    MachPhysMissile* pPhysMissile_;
};

#endif

/* End LMISSILE.HPP *************************************************/
