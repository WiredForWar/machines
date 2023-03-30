/*
 * C O N D D E A D . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogAllUnitsDeadCondition

    This conditon code checks for all units in a given race being dead (i.e. raceObjects is
    empty).

*/

#ifndef _MACHLOG_CONDDEAD_HPP
#define _MACHLOG_CONDDEAD_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogAllUnitsDeadCondition : public SimCondition
// Canonical form revoked
{
public:
    static MachLogAllUnitsDeadCondition* newFromParser(UtlLineTokeniser*);

    MachLogAllUnitsDeadCondition(const string& keyName, MachPhys::Race);

    bool doHasConditionBeenMet() const override;

    ~MachLogAllUnitsDeadCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAllUnitsDeadCondition);
    PER_FRIEND_READ_WRITE(MachLogAllUnitsDeadCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogAllUnitsDeadCondition& t);

    MachLogAllUnitsDeadCondition(const MachLogAllUnitsDeadCondition&);
    MachLogAllUnitsDeadCondition& operator=(const MachLogAllUnitsDeadCondition&);

    MachPhys::Race race_;
};

PER_DECLARE_PERSISTENT(MachLogAllUnitsDeadCondition);

#endif

/* End CONDTIME.HPP *************************************************/
