/*
 * O P T S K C O N . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.


        AI squadron module.
        Designed for constructing buildings - the buildings are placed on the
        construction production list for the AI Controller.
        Everything to do with strategic desicion making
        to do with constructing goes in here... (The actual constructing - not deciding what
        goes into production list).
        (At least thats the plan)
 */

#ifndef _MACHLOG_OPTSKCON_HPP
#define _MACHLOG_OPTSKCON_HPP

#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "machlog/machlog.hpp"
#include "machlog/optsk.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogSquadron;
class MachLogProductionUnit;
class MachLogConstruction;

// canonical form revoked

class MachLogTaskConstructOperation : public MachLogTaskOperation
{
public:
    MachLogTaskConstructOperation(MachLogSquadron* pActor);

    ~MachLogTaskConstructOperation() override;

    static bool couldPlaceConstruction(const MachLogProductionUnit& prod);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogTaskConstructOperation);
    PER_FRIEND_READ_WRITE(MachLogTaskConstructOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;
    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogTaskConstructOperation(const MachLogTaskConstructOperation&);
    MachLogTaskConstructOperation& operator=(const MachLogTaskConstructOperation&);
    bool operator==(const MachLogTaskConstructOperation&);

    bool setCompleteState();
    bool placeConstruction(const MachLogProductionUnit&, MachLogConstruction**);
    void issueConstructOperation(MachLogConstruction*);
    void issueAdminConstructOperation(MachLogConstruction*);

    MachLogSquadron* pActor_;
    bool complete_;
};

PER_DECLARE_PERSISTENT(MachLogTaskConstructOperation);

/* //////////////////////////////////////////////////////////////// */

#endif