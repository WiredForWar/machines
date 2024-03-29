/*
 * C H A R G E R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogSuperCharger

*/

#ifndef _MACHLOG_CHARGER_HPP
#define _MACHLOG_CHARGER_HPP

#include "base/base.hpp"
#include "machlog/weapon.hpp"

class MachPhysSuperCharger;
class MachLogFireData;

class MachLogSuperCharger : public MachLogWeapon
// Canonical form revoked
{
public:
    MachLogSuperCharger(MachLogRace*, MachPhysSuperCharger*, MachActor* pOwner);
    ~MachLogSuperCharger() override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogSuperCharger& t);
    bool healing() const;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData = 0) override;

    static MachLogSuperCharger& asSuperCharger(MachLogWeapon*);

    void stopAllHealing();

    // the persistence mechanism has to reset which weapons go where
    // so we have to allow it to remount the weapons.
    void setPhysicalSuperCharger(MachPhysSuperCharger*);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogSuperCharger);
    PER_FRIEND_READ_WRITE(MachLogSuperCharger);

protected:
    void doFire(MachActor*, const MachLogFireData&) override;

private:
    MachLogSuperCharger(const MachLogSuperCharger&);
    MachLogSuperCharger& operator=(const MachLogSuperCharger&);
    bool operator==(const MachLogSuperCharger&);

    bool healing_;
    PhysAbsoluteTime lastUpdateTime_;
    MachPhysSuperCharger* pPhysSuperCharger_;
};

PER_DECLARE_PERSISTENT(MachLogSuperCharger);

#endif

/* End PULSEWEP.HPP *************************************************/
