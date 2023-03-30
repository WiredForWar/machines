/*
 * O P T S K P A T . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.


        AI squadron module.
        Designed for the generic Patrol squadrons. This squadron task
        may not be limited to the AI - hence do not make assumptions about the
        controller interface.
 */

#ifndef _MACHLOG_OPTSKPAT_HPP
#define _MACHLOG_OPTSKPAT_HPP

#include "ctl/vector.hpp"
#include "phys/phys.hpp"
#include "mathex/point2d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/optsk.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogSquadron;

// canonical form revoked

class MachLogTaskPatrolOperation : public MachLogTaskOperation
{
public:
    using Path = ctl_vector<MexPoint2d>;

    MachLogTaskPatrolOperation(MachLogSquadron* pActor, const Path& path, const PhysRelativeTime& initialDelay);

    ~MachLogTaskPatrolOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTaskPatrolOperation);
    PER_FRIEND_READ_WRITE(MachLogTaskPatrolOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogTaskPatrolOperation(const MachLogTaskPatrolOperation&);
    MachLogTaskPatrolOperation& operator=(const MachLogTaskPatrolOperation&);
    bool operator==(const MachLogTaskPatrolOperation&);

    PhysRelativeTime doUpdateWithCommander();
    PhysRelativeTime doUpdateWithoutCommander();

    MachLogSquadron* pActor_;
    bool complete_;
    Path path_;
    PhysRelativeTime initialDelay_;
    bool initialDelayProcessed_;
    PhysAbsoluteTime startTime_;
};

PER_DECLARE_PERSISTENT(MachLogTaskPatrolOperation);

/* //////////////////////////////////////////////////////////////// */

#endif
