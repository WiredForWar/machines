/*
 * A C T W I N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogExplosionAction

    This action class sets off a generic explosion at given location and of given size.
*/

#ifndef _MACHLOG_ACTExplosion_HPP
#define _MACHLOG_ACTExplosion_HPP

#include "base/base.hpp"
#include "sim/action.hpp"
#include "mathex/point2d.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogExplosionAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogExplosionAction() override;
    static std::unique_ptr<MachLogExplosionAction> newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogExplosionAction);
    PER_FRIEND_READ_WRITE(MachLogExplosionAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogExplosionAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogExplosionAction& t);

    MachLogExplosionAction(const MachLogExplosionAction&);
    MachLogExplosionAction& operator=(const MachLogExplosionAction&);

    MexPoint2d point_;
    MATHEX_SCALAR range_;
    MATHEX_SCALAR size_;
    int damage_;
};

PER_DECLARE_PERSISTENT(MachLogExplosionAction);

#endif

/* End ACTExplosion.HPP *************************************************/
