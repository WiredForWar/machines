/*
 * C O N D T I M E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogAnyAllCondition

    This is a MachLog concrete class to hold the ANY/ALL idea.
*/

#ifndef _MACHLOG_CONDANYA_HPP
#define _MACHLOG_CONDANYA_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "stdlib/strfwd.hpp"

class UtlLineTokeniser;

class MachLogAnyAllCondition : public SimCondition
// Canonical form revoked
{
public:
    using ConditionMap = ctl_map<string, SimCondition*, std::less<string>>;

    static MachLogAnyAllCondition*
    newFromParser(UtlLineTokeniser*, ConditionMap*, SimCondition::BooleanOperator booleanOperator);

    bool doHasConditionBeenMet() const override;

    ~MachLogAnyAllCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAnyAllCondition);
    PER_FRIEND_READ_WRITE(MachLogAnyAllCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogAnyAllCondition(
        const string& keyName,
        const ctl_pvector<SimCondition>&,
        SimCondition::BooleanOperator booleanOperator);

    friend std::ostream& operator<<(std::ostream& o, const MachLogAnyAllCondition& t);

    MachLogAnyAllCondition(const MachLogAnyAllCondition&);
    MachLogAnyAllCondition& operator=(const MachLogAnyAllCondition&);
};

PER_DECLARE_PERSISTENT(MachLogAnyAllCondition);

#endif

/* End CONDTIME.HPP *************************************************/
