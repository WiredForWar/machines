/*
 * A D M I N I S T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACHLOG_ADMINIST_HPP
#define _MACHLOG_ADMINIST_HPP

#include "machlog/machine.hpp"

#include "machlog/canattac.hpp"
#include "machlog/canadmin.hpp"
// #include "machlog/cntrl.hpp"
/* //////////////////////////////////////////////////////////////// */

// class ostream;
class MachPhysAdministrator;
class MachPhysAdministratorData;

class MachLogAdministrator
    : public MachLogMachine
    ,
      //  public MachLogCanPickUp,
      //  public MachLogCanPutDown,
      public MachLogCanAttack
    , public MachLogCanAdminister
// cannonical form revoked
{
public:
    MachLogAdministrator(
        const MachPhys::AdministratorSubType& subType,
        MachLogMachine::Level hwLevel,
        MachLogMachine::Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo wc);

    MachLogAdministrator(
        const MachPhys::AdministratorSubType& subType,
        MachLogMachine::Level hwLevel,
        MachLogMachine::Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo wc,
        UtlId);

    ~MachLogAdministrator() override;

    ///////////////////////////////

    // view of MachPhys data objects
    const MachPhysMachineData& machineData() const override;
    const MachPhysAdministratorData& data() const;

    // inherited from MachLogCanCommunicate
    void receiveMessage(const MachLogMessage& message) override;

    void handleIdleTechnician(MachLogCommsId obj) override;

    const MachPhysAdministrator& physAdministrator() const;
    MachPhysAdministrator& physAdministrator();

    const MachPhys::AdministratorSubType& subType() const;

    // inherited from SimActor and MachActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    // inherited from MachLogMachine
    bool fearsThisActor(const MachActor& otherActor) const override;
    // inherited from MachActor
    int militaryStrength() const override;

    // Amount of damage is passed in beHit()
    void beHit(
        const int&,
        MachPhys::WeaponType byType = MachPhys::N_WEAPON_TYPES,
        MachActor* pByActor = nullptr,
        MexLine3d* pByDirection = nullptr,
        MachActor::EchoBeHit = MachActor::ECHO) override;

    void beHitWithoutAnimation(
        int damage,
        PhysRelativeTime physicalTimeDelay = 0,
        MachActor* pByActor = nullptr,
        EchoBeHit = ECHO) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAdministrator);
    PER_FRIEND_READ_WRITE(MachLogAdministrator);

private:
    MachLogAdministrator(const MachLogAdministrator&);
    MachLogAdministrator& operator=(const MachLogAdministrator&);
    bool operator==(const MachLogAdministrator&) const;

    static MachPhysAdministrator* pNewPhysAdministrator(
        const MachPhys::AdministratorSubType& subType,
        Level hwLevel,
        Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo);

    void doOutputOperator(std::ostream& o) const override;
    MachPhys::AdministratorSubType subType_;
};

PER_DECLARE_PERSISTENT(MachLogAdministrator);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACHLOG_ADMINIST_HPP   */

/* End ADMINIST.HPP *************************************************/