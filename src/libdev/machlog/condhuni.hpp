/*
 * C O N D T I M E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogHighUnitCountCondition

    This conditon code checks for the existence (on a race basis) for a certain number of the specified
    ObjectType - this pulls directly into the MLRaces collections.
    ObjectTypes allowed Machines heirarchy, construction hierarchy

*/

#ifndef _MACHLOG_CONDHIGH_UNIT_HPP
#define _MACHLOG_CONDHIGH_UNIT_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogHighUnitCountCondition : public SimCondition
// Canonical form revoked
{
public:
    static MachLogHighUnitCountCondition* newFromParser(UtlLineTokeniser*);

    // this constructor will not normally be called directly but may be called for the dynamic creation of these
    // objects.
    MachLogHighUnitCountCondition(
        const std::string& keyName,
        MachPhys::Race,
        MachLog::ObjectType,
        size_t number,
        int subType);

    bool doHasConditionBeenMet() const override;

    ~MachLogHighUnitCountCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogHighUnitCountCondition);
    PER_FRIEND_READ_WRITE(MachLogHighUnitCountCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    // MachLogHighUnitCountCondition( const string& keyName, MachPhys::Race, MachLog::ObjectType, size_t number, int
    // subType );

    friend std::ostream& operator<<(std::ostream& o, const MachLogHighUnitCountCondition& t);

    MachLogHighUnitCountCondition(const MachLogHighUnitCountCondition&);
    MachLogHighUnitCountCondition& operator=(const MachLogHighUnitCountCondition&);

    MachLog::ObjectType objectType_;
    MachPhys::Race race_;
    size_t number_;
    // the sub type is only valid for artefacts
    int subType_;
};

PER_DECLARE_PERSISTENT(MachLogHighUnitCountCondition);

#endif

/* End CONDTIME.HPP *************************************************/
