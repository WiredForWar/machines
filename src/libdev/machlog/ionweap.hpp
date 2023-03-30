/*
 * V O R T W E A P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogIonWeapon

*/

#ifndef _MACHLOG_IONWEAP_HPP
#define _MACHLOG_IONWEAP_HPP

#include "base/base.hpp"
#include "machlog/weapon.hpp"

class MachLogFireData;

class MachLogIonWeapon : public MachLogWeapon
// Canonical form revoked
{
public:
    MachLogIonWeapon(MachLogRace*, MachPhysWeapon*, MachActor* pOwner);
    ~MachLogIonWeapon() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogIonWeapon& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogIonWeapon);
    PER_FRIEND_READ_WRITE(MachLogIonWeapon);

protected:
    void doFire(MachActor*, const MachLogFireData&) override;
    void doFire(const MexPoint3d& position) override;

private:
    MachLogIonWeapon(const MachLogIonWeapon&);
    MachLogIonWeapon& operator=(const MachLogIonWeapon&);
    bool operator==(const MachLogIonWeapon&);
};

PER_DECLARE_PERSISTENT(MachLogIonWeapon);

#endif

/* End IONWEAP.HPP *************************************************/
