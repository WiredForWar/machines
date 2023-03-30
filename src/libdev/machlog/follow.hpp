/*
 * F O L L O W . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachFollowOperation

    Causes the machine to follow another machine
*/

// Orthodox canonical (revoked)

#ifndef _MACHLOG_FOLLOW_HPP
#define _MACHLOG_FOLLOW_HPP

#include "base/base.hpp"

#include "phys/phys.hpp"
#include "machlog/operatio.hpp"

// Forward declarations
class MachLogMachine;
class MexPoint2d;
class MachLogFollowOperationImpl;

class MachLogFollowOperation : public MachLogOperation
{
public:
    enum CamouflagedOp
    {
        CAMOUFLAGED,
        NOT_CAMOUFLAGED
    };
    enum TerminateFlag
    {
        TERMINATE_WHEN_CLOSE,
        DONT_TERMINATE
    };

    // ctor. pActor follows pMachine at relative position offset
    MachLogFollowOperation(
        MachLogMachine* pActor,
        MachLogMachine* pTarget,
        const MexPoint2d& offset,
        CamouflagedOp camoStatus = NOT_CAMOUFLAGED,
        TerminateFlag terminateFlag = DONT_TERMINATE);

    MachLogFollowOperation(
        MachLogMachine* pActor,
        MachLogMachine* pTarget,
        const MexPoint2d& offset,
        PhysPathFindingPriority,
        CamouflagedOp camoStatus = NOT_CAMOUFLAGED,
        TerminateFlag terminateFlag = DONT_TERMINATE);

    // dtor
    ~MachLogFollowOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogFollowOperation);
    PER_FRIEND_READ_WRITE(MachLogFollowOperation);

protected:
    bool doStart() override;
    PhysRelativeTime doUpdate() override;
    void doFinish() override;

    bool doIsFinished() const override;

    void doOutputOperator(std::ostream&) const override;

    bool doBeInterrupted() override;

    void CLASS_INVARIANT;

private:
    // Operations deliberately revoked
    MachLogFollowOperation(const MachLogFollowOperation&);
    MachLogFollowOperation& operator=(const MachLogFollowOperation&);
    bool operator==(const MachLogFollowOperation&);

    MachLogMachine* pTarget();

    // data members
    MachLogFollowOperationImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogFollowOperation);
PER_ENUM_PERSISTENT(MachLogFollowOperation::CamouflagedOp);
PER_ENUM_PERSISTENT(MachLogFollowOperation::TerminateFlag);

#endif

/* End FOLLOW.HPP ***************************************************/
