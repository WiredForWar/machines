/*
 * A G G R E S S R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
 * MachLogAggressor
 * Implements the logical behaviour of Aggressors.
 */

#ifndef _MACHLOG_AGGRESSR_HPP
#define _MACHLOG_AGGRESSR_HPP

#include "mathex/radians.hpp"

#include "machlog/canattac.hpp"
#include "machlog/machine.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachPhysAggressor;
class MachPhysAggressorData;

class MachLogAggressor
    : public MachLogMachine
    , public MachLogCanAttack
// canonical form revoked
{
public:
    MachLogAggressor(
        const MachPhys::AggressorSubType& subType,
        Level hwLevel,
        Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo);

    MachLogAggressor(
        const MachPhys::AggressorSubType& subType,
        Level hwLevel,
        Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo,
        UtlId withId);

    ~MachLogAggressor() override {};

    ///////////////////////////////

    // view of MachPhys data objects
    const MachPhysMachineData& machineData() const override;
    const MachPhysAggressorData& data() const;

    ///////////////////////////////

    void CLASS_INVARIANT;

    // inherited from SimActor and MachActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    ///////////////////////////////

    const MachPhysAggressor& physAggressor() const;
    MachPhysAggressor& physAggressor();

    const MachPhys::AggressorSubType& subType() const;

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

    // useful for eradicator-specific logic
    bool isEradicator() const;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAggressor);
    PER_FRIEND_READ_WRITE(MachLogAggressor);

private:
    static MachPhysAggressor* pNewPhysAggressor(
        const MachPhys::AggressorSubType& subType,
        Level hwLevel,
        Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        MachPhys::WeaponCombo);

    MachLogAggressor(const MachLogAggressor&);
    MachLogAggressor& operator=(const MachLogAggressor&);

    ///////////////////////////////

    MachPhys::AggressorSubType subType_;

    ///////////////////////////////
};

PER_DECLARE_PERSISTENT(MachLogAggressor);
/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACHLOG_AGGRESSR_HPP   */

/* End AGGRESSR.HPP *************************************************/
