/*
 * O P A D L O C . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#ifndef _MACHLOG_OPADLOC_HPP
#define _MACHLOG_OPADLOC_HPP

#include "ctl/list.hpp"

#include "phys/phys.hpp"

#include "mathex/point2d.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/operatio.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogAdministrator;
class MachLogGeoLocator;
class MachLogAdminLocateOperationImpl;

// orthodox canonical (revoked)

class MachLogAdminLocateOperation : public MachLogOperation
{
public:
    using Path = ctl_list<MexPoint2d>;
    // CTORs are used as follows:
    // given to the AI controllers will go and find ore by cheating
    MachLogAdminLocateOperation(MachLogAdministrator* pActor);
    // single point
    MachLogAdminLocateOperation(MachLogAdministrator* pActor, const MexPoint3d& dest);
    // way point path
    MachLogAdminLocateOperation(MachLogAdministrator* pActor, const Path& externalPath);

    ~MachLogAdminLocateOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogAdminLocateOperation);
    PER_FRIEND_READ_WRITE(MachLogAdminLocateOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogAdminLocateOperation(const MachLogAdminLocateOperation&);
    MachLogAdminLocateOperation& operator=(const MachLogAdminLocateOperation&);
    bool operator==(const MachLogAdminLocateOperation&);

    void assignLocatorTask(MachLogGeoLocator*);

    MachLogAdminLocateOperationImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogAdminLocateOperation);

/* //////////////////////////////////////////////////////////////// */

#endif