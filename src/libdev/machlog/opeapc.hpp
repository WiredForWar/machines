/*
 * O P E A P C . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogEnterAPCOperation

    This operation will be applied to a machine wishing to enter
    an APC.
*/

#ifndef _MACHLOG_OPEAPC_HPP
#define _MACHLOG_OPEAPC_HPP

#include "base/base.hpp"
#include "machlog/operatio.hpp"

class MachLogMachine;
class MachLogAPC;

// orthodox canonical (revoked)

class MachLogEnterAPCOperation : public MachLogOperation
{
public:
    MachLogEnterAPCOperation(MachLogMachine*, MachLogAPC*);
    MachLogEnterAPCOperation(MachLogMachine*, MachLogAPC*, PhysPathFindingPriority);

    ~MachLogEnterAPCOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogEnterAPCOperation);
    PER_FRIEND_READ_WRITE(MachLogEnterAPCOperation);

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

    friend std::ostream& operator<<(std::ostream& o, const MachLogEnterAPCOperation& t);

private:
    // Operations deliberately revoked
    MachLogEnterAPCOperation(const MachLogEnterAPCOperation&);
    MachLogEnterAPCOperation& operator=(const MachLogEnterAPCOperation&);
    bool operator==(const MachLogEnterAPCOperation&);

    enum Status
    {
        PREPARING_TO_GO_TO_APC,
        GOING_TO_APC,
        ENTERING_APC
    };
    PER_FRIEND_ENUM_PERSISTENT(Status);

    void checkAndDoStopGoingToAPC();

    // data members
    MachLogMachine* pActor_;
    MachLogAPC* pAPC_;
    bool finished_;
    MATHEX_SCALAR desiredRange_;
    Status status_;
};

PER_DECLARE_PERSISTENT(MachLogEnterAPCOperation);

#endif

/* End OPEAPC.HPP ***************************************************/
