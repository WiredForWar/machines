/*
 * A C T TASKS . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogSetFlagAction

    This action class updates the state of a task in the curent scenario
*/

#ifndef _MACHLOG_ACTTASKS_HPP
#define _MACHLOG_ACTTASKS_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "sim/action.hpp"
#include "machphys/machphys.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogTaskStateAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogTaskStateAction() override;
    static std::unique_ptr<MachLogTaskStateAction> newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);
    static std::unique_ptr<MachLogTaskStateAction>
    newDynamic(SimCondition*, bool enabled, uint taskIndex, bool isAvailabe, bool isComplete);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTaskStateAction);
    PER_FRIEND_READ_WRITE(MachLogTaskStateAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogTaskStateAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogTaskStateAction& t);

    MachLogTaskStateAction(const MachLogTaskStateAction&);
    MachLogTaskStateAction& operator=(const MachLogTaskStateAction&);

    uint taskIndex_; // Number of the task, (starts counting at 1)
    bool isAvailable_; // When the user is to b einformed of it
    bool isComplete_; // When he has completed it
};

PER_DECLARE_PERSISTENT(MachLogTaskStateAction);

#endif

/* End ACTTASKS.HPP *************************************************/
