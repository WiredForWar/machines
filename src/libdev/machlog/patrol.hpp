/*
 * P A T R O L . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#ifndef _MACHLOG_PATROL_HPP
#define _MACHLOG_PATROL_HPP

#include "ctl/vector.hpp"

#include "mathex/point2d.hpp"

#include "machlog/animatio.hpp"
#include "machlog/operatio.hpp"

/* //////////////////////////////////////////////////////////////// */

// forward refs
class MachLogAdministrator;
class MachLogAggressor;
class MachLogMachine;
class MachActor;

class MachLogPatrolOperation : public MachLogOperation
{
public:
    using Path = ctl_vector<MexPoint2d>;

    MachLogPatrolOperation(MachLogMachine* pActor, const Path& path, const bool& loop);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogPatrolOperation);
    PER_FRIEND_READ_WRITE(MachLogPatrolOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;
    bool doBeInterrupted() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogMachine* pActor_;
    Path path_;
    int pathElement_;
    bool finished_;
    bool loop_;
};

PER_DECLARE_PERSISTENT(MachLogPatrolOperation);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACHLOG_PATROL_HPP */

/* End PATROL.HPP ***************************************************/