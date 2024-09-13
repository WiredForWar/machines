/*
 * A C T FLAG . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogSetFlagAction

    This action class sets a scenario flag which is saved in the player database
*/

#ifndef _MACHLOG_ACTFLAG_HPP
#define _MACHLOG_ACTFLAG_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "sim/action.hpp"
#include "machphys/machphys.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogSetFlagAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogSetFlagAction() override;
    static std::unique_ptr<MachLogSetFlagAction> newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);
    static std::unique_ptr<MachLogSetFlagAction> newDynamic(SimCondition*, bool enabled, const std::string& flag);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogSetFlagAction);
    PER_FRIEND_READ_WRITE(MachLogSetFlagAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogSetFlagAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogSetFlagAction& t);

    MachLogSetFlagAction(const MachLogSetFlagAction&);
    MachLogSetFlagAction& operator=(const MachLogSetFlagAction&);

    std::string flagName_; // Name of the flag to be set
};

PER_DECLARE_PERSISTENT(MachLogSetFlagAction);

#endif

/* End ACTFLAG.HPP *************************************************/
