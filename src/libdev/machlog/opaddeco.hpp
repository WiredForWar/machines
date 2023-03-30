/*
 * O P A D D C O N . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#ifndef _MACHLOG_OPADDECO_HPP
#define _MACHLOG_OPADDECO_HPP

#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/operatio.hpp"

class MachLogAdministrator;
class MachLogConstructor;
class MachLogResourceCarrier;
class MachLogConstruction;

// orthodox canonical (revoked)

class MachLogAdminDeconstructOperation : public MachLogOperation
{
public:
    MachLogAdminDeconstructOperation(MachLogAdministrator* pActor, MachLogConstruction*);

    ~MachLogAdminDeconstructOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAdminDeconstructOperation);
    PER_FRIEND_READ_WRITE(MachLogAdminDeconstructOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogAdminDeconstructOperation(const MachLogAdminDeconstructOperation&);
    MachLogAdminDeconstructOperation& operator=(const MachLogAdminDeconstructOperation&);
    bool operator==(const MachLogAdminDeconstructOperation&);

    MachLogAdministrator* pActor_;
    MachLogConstruction* pConstruction_;

    bool complete_;
    bool currentlyAttached_;
};

PER_DECLARE_PERSISTENT(MachLogAdminDeconstructOperation);

#endif