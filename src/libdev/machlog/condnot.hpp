/*
 * C O N D T I M E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogNotCondition

    This is a MachLog concrete class to hold the NOT idea.
*/

#ifndef _MACHLOG_CONDNOT_HPP
#define _MACHLOG_CONDNOT_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "stdlib/strfwd.hpp"

class UtlLineTokeniser;
template <class X, class Y, class Z> class ctl_map;
template <class T> class less;

class MachLogNotCondition : public SimCondition
// Canonical form revoked
{
public:
    using ConditionMap = ctl_map<string, SimCondition*, std::less<string>>;

    static MachLogNotCondition* newFromParser(UtlLineTokeniser*, ConditionMap*);

    bool doHasConditionBeenMet() const override;

    ~MachLogNotCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogNotCondition);
    PER_FRIEND_READ_WRITE(MachLogNotCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogNotCondition(const string& keyName, SimCondition*);

    friend std::ostream& operator<<(std::ostream& o, const MachLogNotCondition& t);

    MachLogNotCondition(const MachLogNotCondition&);
    MachLogNotCondition& operator=(const MachLogNotCondition&);

    SimCondition* pNotCondition_;
};

PER_DECLARE_PERSISTENT(MachLogNotCondition);

#endif

/* End CONDTIME.HPP *************************************************/
