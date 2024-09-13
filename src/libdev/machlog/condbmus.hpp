/*
 * C O N D T I M E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogBMUCountCondition

    This conditon code checks for the BMUs being at a certain level it can cope with greater than or less than
    checks.

*/

#ifndef _MACHLOG_CONDBMUS_HPP
#define _MACHLOG_CONDBMUS_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogBMUCountCondition : public SimCondition
// Canonical form revoked
{
public:
    enum CheckType
    {
        LESS_THAN_EQUAL,
        GREATER_THAN_EQUAL
    };

    static MachLogBMUCountCondition* newFromParser(UtlLineTokeniser*);

    bool doHasConditionBeenMet() const override;

    ~MachLogBMUCountCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogBMUCountCondition);
    PER_FRIEND_READ_WRITE(MachLogBMUCountCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogBMUCountCondition(const std::string& keyName, MachPhys::Race, MachPhys::BuildingMaterialUnits, CheckType);

    friend std::ostream& operator<<(std::ostream& o, const MachLogBMUCountCondition& t);

    MachLogBMUCountCondition(const MachLogBMUCountCondition&);
    MachLogBMUCountCondition& operator=(const MachLogBMUCountCondition&);

    MachPhys::Race race_;
    // the number to check against
    MachPhys::BuildingMaterialUnits number_;
    CheckType checkType_;
};

PER_DECLARE_PERSISTENT(MachLogBMUCountCondition);
PER_ENUM_PERSISTENT(MachLogBMUCountCondition::CheckType);

#endif

/* End CONDTIME.HPP *************************************************/
