/*
 * C O N T R O L R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
 * MachLogPCController represents one of the PC UI/ML interface
 * in the logical command heirarchy.
 */

#ifndef _MACHLOG_CNTRL_PC_HPP
#define _MACHLOG_CNTRL_PC_HPP

#include "sim/actor.hpp"

#include "machphys/machphys.hpp"

#include "machlog/cntrl.hpp"
#include "machlog/message.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogPCController : public MachLogController
{
public:
    enum SquadronType
    {
        NO_SQUAD,
        LOCATOR_SQUAD,
        CONSTRUCTION_SQUAD,
        PATROL_SQUAD,
        KILLER_SQUAD,
        N_SQUAD_TYPES
    };

    MachLogPCController(MachLogRace* logRace, W4dEntity* pPhysObject);

    ~MachLogPCController() override;

    // inherited from MachLogCanCommunicate...
    void sendMessage(MachLogCommsId to, const MachLogMessage&) const override;
    void receiveMessage(const MachLogMessage&) override;

    // inherited from MachLogCanAdminister
    void handleIdleTechnician(MachLogCommsId) override;
    void handleIdleAdministrator(MachLogCommsId) override;
    void handleIdleAggressor(MachLogCommsId) override;
    void handleIdleAPC(MachLogCommsId) override;
    void handleIdleResourceCarrier(MachLogCommsId) override;
    void handleIdleConstructor(MachLogCommsId) override;
    void handleIdleGeoLocator(MachLogCommsId) override;
    void handleIdleSpyLocator(MachLogCommsId) override;

    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR) override;

    void doOutputOperator(std::ostream& o) const override;

    friend std::ostream& operator<<(std::ostream&, const MachLogPCController&);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogPCController);
    PER_FRIEND_READ_WRITE(MachLogPCController);

private:
};

PER_DECLARE_PERSISTENT(MachLogPCController);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACHLOG_CONTRL_PC_HPP  */

/* End CONTROLR.HPP *************************************************/
