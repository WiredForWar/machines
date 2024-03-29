/*
 * O P S T A N D G . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogStandGroundOperation

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_OPSTANDG_HPP
#define _MACHLOG_OPSTANDG_HPP

#include "base/base.hpp"
#include "machlog/operatio.hpp"

class MachLogMachine;

// canonical form revoked

class MachLogStandGroundOperation : public MachLogOperation
{
public:
    MachLogStandGroundOperation(MachLogMachine*);
    ~MachLogStandGroundOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogStandGroundOperation);
    PER_FRIEND_READ_WRITE(MachLogStandGroundOperation);

protected:
    bool doStart() override;
    // PRE( not isFinished() );
    void doFinish() override;
    // PRE( isFinished() );

    bool doIsFinished() const override;

    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;

    bool doBeInterrupted() override;
    ///////////////////////////////

private:
    // Operations deliberately revoked
    MachLogStandGroundOperation(const MachLogStandGroundOperation&);
    MachLogStandGroundOperation& operator=(const MachLogStandGroundOperation&);
    bool operator==(const MachLogStandGroundOperation&);

    MachLogMachine* pActor_;
};

PER_DECLARE_PERSISTENT(MachLogStandGroundOperation);

#endif

/* End OPSTANDG.HPP *************************************************/
