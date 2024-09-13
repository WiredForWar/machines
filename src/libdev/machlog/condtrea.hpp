/*
 * C O N D T R E A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogTechniciansTreacheriedCondition

    This conditon code checks for a certain number of technicians having been treacheried on this scenario.

*/

#ifndef _MACHLOG_CONDTREA_HPP
#define _MACHLOG_CONDTREA_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogTechniciansTreacheriedCondition : public SimCondition
// Canonical form revoked
{
public:
    static MachLogTechniciansTreacheriedCondition* newFromParser(UtlLineTokeniser*);

    bool doHasConditionBeenMet() const override;

    ~MachLogTechniciansTreacheriedCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTechniciansTreacheriedCondition);
    PER_FRIEND_READ_WRITE(MachLogTechniciansTreacheriedCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogTechniciansTreacheriedCondition(const std::string& keyName, MachPhys::Race, int number);

    friend std::ostream& operator<<(std::ostream& o, const MachLogTechniciansTreacheriedCondition& t);

    MachLogTechniciansTreacheriedCondition(const MachLogTechniciansTreacheriedCondition&);
    MachLogTechniciansTreacheriedCondition& operator=(const MachLogTechniciansTreacheriedCondition&);

    MachPhys::Race race_;
    int number_;
};

PER_DECLARE_PERSISTENT(MachLogTechniciansTreacheriedCondition);

#endif

/* End CONDTIME.HPP *************************************************/
