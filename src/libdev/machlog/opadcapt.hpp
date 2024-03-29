/*
 * O P A D C A P T . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#ifndef _MACHLOG_OPADCAPT_HPP
#define _MACHLOG_OPADCAPT_HPP

#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/operatio.hpp"

class MachLogAdministrator;
class MachLogConstructor;
class MachLogResourceCarrier;
class MachLogConstruction;

// orthodox canonical revoked

class MachLogAdminCaptureOperation : public MachLogOperation
{
public:
    MachLogAdminCaptureOperation(MachLogAdministrator* pActor, MachLogConstruction*);

    ~MachLogAdminCaptureOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAdminCaptureOperation);
    PER_FRIEND_READ_WRITE(MachLogAdminCaptureOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogAdminCaptureOperation(const MachLogAdminCaptureOperation&);
    MachLogAdminCaptureOperation& operator=(const MachLogAdminCaptureOperation&);
    bool operator==(const MachLogAdminCaptureOperation&);

    MachLogAdministrator* pActor_;
    MachLogConstruction* pConstruction_;

    bool complete_;
    bool currentlyAttached_;
};

PER_DECLARE_PERSISTENT(MachLogAdminCaptureOperation);

#endif