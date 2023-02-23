/*
 * O P T S K L O C . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.


        AI squadron module.
        Designed for locating mineral sites and the such like.
        Everything to do with strategic desicion making
        to do with Locating goes in here...
        (At least thats the plan)
 */

#ifndef _MACHLOG_OPTSKLOC_HPP
#define _MACHLOG_OPTSKLOC_HPP

#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/optsk.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogSquadron;

// canonical form revoked

class MachLogTaskLocateOperation : public MachLogTaskOperation
{
public:
    // CTORs are used as follows:
    // given to the AI controllers will go and find ore by cheating
    MachLogTaskLocateOperation(MachLogSquadron* pActor, int nTotalDesiredSites, MATHEX_SCALAR maxRangeFromPod);

    ~MachLogTaskLocateOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTaskLocateOperation);
    PER_FRIEND_READ_WRITE(MachLogTaskLocateOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogTaskLocateOperation(const MachLogTaskLocateOperation&);
    MachLogTaskLocateOperation& operator=(const MachLogTaskLocateOperation&);
    bool operator==(const MachLogTaskLocateOperation&);

    bool setCompleteState();

    MachLogSquadron* pActor_;
    bool complete_;
    MATHEX_SCALAR maxRangeFromPod_;
    int nDesiredSites_;
    int totalWeDiscovered_;
    int totalDiscovered_;
    int totalPossibleToDiscover_;
};

PER_DECLARE_PERSISTENT(MachLogTaskLocateOperation);

/* //////////////////////////////////////////////////////////////// */

#endif