/*
 * A C T S Q U A D . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogActivateSquadronsAction

    This action class will set the delay on TaskAttackOperations to zero
    for each of the squadrons named assuming that the squadrons have a taskAttackOperation.
    If they do not then an assertion will occur in the debug version.
*/

#ifndef _MACHLOG_ACTActivateSquadrons_HPP
#define _MACHLOG_ACTActivateSquadrons_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "sim/action.hpp"
#include "machphys/machphys.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogActivateSquadronsAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogActivateSquadronsAction() override;
    static std::unique_ptr<MachLogActivateSquadronsAction>
    newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogActivateSquadronsAction);
    PER_FRIEND_READ_WRITE(MachLogActivateSquadronsAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogActivateSquadronsAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogActivateSquadronsAction& t);

    MachLogActivateSquadronsAction(const MachLogActivateSquadronsAction&);
    MachLogActivateSquadronsAction& operator=(const MachLogActivateSquadronsAction&);

    MachPhys::Race race_;
    // the squadrons are held by id and not by pointer as the squdrons will not be created by this time
    using Squadrons = ctl_vector<int>;
    Squadrons squadrons_;
};

PER_DECLARE_PERSISTENT(MachLogActivateSquadronsAction);

#endif

/* End ACTActivateSquadrons.HPP *************************************************/
