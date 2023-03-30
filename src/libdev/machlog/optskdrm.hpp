/*
 * O P T S K D R M . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.


        AI squadron module.
        Designed for using spys to drop mines in various different types of shape.
 */

#ifndef _MACHLOG_OPTSKDRM_HPP
#define _MACHLOG_OPTSKDRM_HPP

#include "phys/phys.hpp"
#include "mathex/point2d.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/optsk.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogSquadron;

// canonical form revoked

class MachLogTaskDropLandMineOperation : public MachLogTaskOperation
{
public:
    MachLogTaskDropLandMineOperation(MachLogSquadron* pActor, MATHEX_SCALAR minRange, MATHEX_SCALAR maxRange);
    MachLogTaskDropLandMineOperation(
        MachLogSquadron* pActor,
        MATHEX_SCALAR minRange,
        MATHEX_SCALAR maxRange,
        const MexPoint2d& start);

    ~MachLogTaskDropLandMineOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTaskDropLandMineOperation);
    PER_FRIEND_READ_WRITE(MachLogTaskDropLandMineOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogTaskDropLandMineOperation(const MachLogTaskDropLandMineOperation&);
    MachLogTaskDropLandMineOperation& operator=(const MachLogTaskDropLandMineOperation&);
    bool operator==(const MachLogTaskDropLandMineOperation&);

    bool setCompleteState();

    MachLogSquadron* pActor_;
    bool complete_;
    MATHEX_SCALAR minRange_;
    MATHEX_SCALAR maxRange_;
    MexPoint2d start_;
    bool startDefined_;
};

PER_DECLARE_PERSISTENT(MachLogTaskDropLandMineOperation);

/* //////////////////////////////////////////////////////////////// */

#endif