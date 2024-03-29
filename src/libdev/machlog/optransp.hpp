/*
 * O P T R A N S P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogTransportOperation

    This class describes the transport operation. Transport is defined as a (number)
    of PickUp operations followed by a putDown operation at a smelter.
    If the pick up point(s) is a mine then a supply route will be set up until that
    mine ceases work.
*/

#ifndef _MACHLOG_OPTRANSP_HPP
#define _MACHLOG_OPTRANSP_HPP

#include "base/base.hpp"

#include "machlog/operatio.hpp"
#include "machlog/rescarr.hpp"

class MachActor;
class MachLogSmelter;
class MachLogTransportOperationImpl;

template <class T> class ctl_pvector;

// canonical form revoked

class MachLogTransportOperation : public MachLogOperation
{
public:
    MachLogTransportOperation(MachLogResourceCarrier*, MachLogResourceCarrier::StartWithPickUpOrPutDown);
    // PRE( pActor_->isNormalResourceCarrier() );

    ~MachLogTransportOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTransportOperation);
    PER_FRIEND_READ_WRITE(MachLogTransportOperation);

protected:
    bool doStart() override;
    // PRE( not isFinished() );
    void doFinish() override;
    // PRE( isFinished() );

    bool doIsFinished() const override;

    void doOutputOperator(std::ostream&) const override;

    PhysRelativeTime doUpdate() override;

    bool doBeInterrupted() override;
    ///////////////////////////////
    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogTransportOperation& t);

private:
    // Operations deliberately revoked
    MachLogTransportOperation(const MachLogTransportOperation&);
    MachLogTransportOperation& operator=(const MachLogTransportOperation&);
    bool operator==(const MachLogTransportOperation&);

    // data members
    MachLogTransportOperationImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogTransportOperation);

/* End OPTRANSP.HPP *************************************************/
#endif