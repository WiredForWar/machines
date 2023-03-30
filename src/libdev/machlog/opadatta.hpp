/*
 * O P A D A T T A . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 *
 *  Administrator attack operation. mainly for use with squadrons.
 *  This operation will move machines in a convoy centred around the
 *  Administrator and then attack whatever the target is.
 */

// Orthodox canonical (revoked)

#ifndef _MACHLOG_OPADATTA_HPP
#define _MACHLOG_OPADATTA_HPP

#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/operatio.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogAdministrator;
class MachActor;

class MachLogAdminAttackOperation : public MachLogOperation
{
public:
    MachLogAdminAttackOperation(MachLogAdministrator* pActor, MachActor* pDirectObject);
    MachLogAdminAttackOperation(MachLogAdministrator* pActor, MachActor* pDirectObject, PhysPathFindingPriority);
    ~MachLogAdminAttackOperation() override;
    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAdminAttackOperation);
    PER_FRIEND_READ_WRITE(MachLogAdminAttackOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogAdminAttackOperation(const MachLogAdminAttackOperation&);
    MachLogAdminAttackOperation& operator=(const MachLogAdminAttackOperation&);
    bool operator==(const MachLogAdminAttackOperation&);

    MachLogAdministrator* pActor_;
    MachActor* pDirectObject_;
    bool complete_;
    bool currentlyAttached_;
    MexPoint2d lastTargetPosition_;
    //  MexPoint3d          dest_;
};

PER_DECLARE_PERSISTENT(MachLogAdminAttackOperation);

/* //////////////////////////////////////////////////////////////// */

#endif