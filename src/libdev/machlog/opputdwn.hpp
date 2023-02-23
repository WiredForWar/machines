/*
 * O P P U T D W N . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogPutDownOperation

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_OPPUTDWN_HPP
#define _MACHLOG_OPPUTDWN_HPP

#include "base/base.hpp"

#include "machlog/operatio.hpp"

class MachLogResourceCarrier;
class MachLogPutDownOperationImpl;
class MexPoint3d;

// canonical form revoked

class MachLogPutDownOperation : public MachLogOperation
{
public:
    MachLogPutDownOperation(MachLogResourceCarrier* pActor);
    // PRE( pActor_->isNormalResourceCarrier() );

    ~MachLogPutDownOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogPutDownOperation);
    PER_FRIEND_READ_WRITE(MachLogPutDownOperation);

protected:
    bool doStart() override;
    // PRE( not isFinished() );
    void doFinish() override;
    // PRE( isFinished() );

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(ostream&) const override;

    bool doBeInterrupted() override;
    ///////////////////////////////

private:
    // Operations deliberately revoked
    MachLogPutDownOperation(const MachLogPutDownOperation&);
    MachLogPutDownOperation& operator=(const MachLogPutDownOperation&);
    bool operator==(const MachLogPutDownOperation&);

    // updates destination to nearest unoccupied pad (chooses pad at random if all pads are occupied)
    void chooseMostViablePad();

    // updates destination pad, and cancels current subop and movement if destination pad is now different
    void possiblyChangeDestinationPad();

    // will perform a find space and change *pClearPutDownPoint if necessary to that of a position near its original
    // value that is guaranteed to be both within putdown range of the pad and clear of all obstacles for the clearance
    // designated
    bool attemptToGuaranteeClearPutDownPoint(
        const MexPoint2d& actorPos,
        MATHEX_SCALAR carrierClearance,
        MexPoint2d* pClearPutDownPoint);

    // data members
    MachLogPutDownOperationImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogPutDownOperation);

#endif

/* End OPPUTDWN.HPP *************************************************/
