/*
 * O P L M I N E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogRefillLandMinesOperation

    operation for spy locators which makes the spy go to the nearest garrison and recharge mines.
*/

#ifndef _MACHLOG_OPRefillLandMines_HPP
#define _MACHLOG_OPRefillLandMines_HPP

#include "base/base.hpp"
#include "mathex/point3d.hpp"
#include "machlog/operatio.hpp"

class MachLogSpyLocator;
class MachLogGarrison;

// canonical form revoked

class MachLogRefillLandMinesOperation : public MachLogOperation
{
public:
    MachLogRefillLandMinesOperation(MachLogSpyLocator* pActor);
    MachLogRefillLandMinesOperation(MachLogSpyLocator* pActor, MachLogGarrison* pGarrison);

    ~MachLogRefillLandMinesOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogRefillLandMinesOperation);
    PER_FRIEND_READ_WRITE(MachLogRefillLandMinesOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    bool doBeInterrupted() override;

    void doOutputOperator(ostream&) const override;

    // redefinition required, invocation revoked
    PhysRelativeTime doUpdate() override;

private:
    // Operations deliberately revoked
    MachLogRefillLandMinesOperation(const MachLogRefillLandMinesOperation&);
    MachLogRefillLandMinesOperation& operator=(const MachLogRefillLandMinesOperation&);
    bool operator==(const MachLogRefillLandMinesOperation&);

    MachLogSpyLocator* pActor_;
    MachLogGarrison* pGarrison_;
    bool complete_;
    bool clientSuppliedGarrison_;
    MexPoint3d dest_;
    bool doneMove_;
};

PER_DECLARE_PERSISTENT(MachLogRefillLandMinesOperation);

#endif

/* End OPRefillLandMines.HPP *************************************************/
