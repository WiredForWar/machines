/*
 * E P P s. C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "machphys/Locomotion/EvasionPriorityPlans.hpp"
#include "machphys/Locomotion/EvasionPriorityPlan.hpp"

MachPhysEvasionPriorityPlans& MachPhysEvasionPriorityPlans::instance()
{
    static MachPhysEvasionPriorityPlans instance_;
    return instance_;
}

void MachPhysEvasionPriorityPlans::clear()
{
    EPPs_.clear();
}

void MachPhysEvasionPriorityPlans::addNewEPP(const std::string& EPPName)
{
    PRE(!EPPExists(EPPName));

    EPPs_.push_back(std::make_unique<MachPhysEvasionPriorityPlan>(EPPName));
}

void MachPhysEvasionPriorityPlans::garrisonPriority(const std::string& EPPName, int priority)
{
    PRE_INFO(priority);
    // Special value of -2 allowed for garrisons which will indicate that NO evasion is ever to be
    // undertaken by any machine with this priority plan.
    PRE(priority < N_PRIORITY_LEVELS && priority >= -2);
    PRE(EPPExists(EPPName));

    EPP(EPPName).garrisonPriority(priority);
}

void MachPhysEvasionPriorityPlans::aggressivesPriority(const std::string& EPPName, int priority)
{
    PRE_INFO(priority);
    PRE(priority < N_PRIORITY_LEVELS && priority >= -1);
    PRE(EPPExists(EPPName));

    EPP(EPPName).aggressivesPriority(priority);
}

void MachPhysEvasionPriorityPlans::podPriority(const std::string& EPPName, int priority)
{
    PRE_INFO(priority);
    PRE(priority < N_PRIORITY_LEVELS && priority >= -1);
    PRE(EPPExists(EPPName));

    EPP(EPPName).podPriority(priority);
}

void MachPhysEvasionPriorityPlans::turretsPriority(const std::string& EPPName, int priority)
{
    PRE_INFO(priority);
    PRE(priority < N_PRIORITY_LEVELS && priority >= -1);
    PRE(EPPExists(EPPName));

    EPP(EPPName).turretsPriority(priority);
}

int MachPhysEvasionPriorityPlans::garrisonPriority(const std::string& EPPName) const
{
    PRE(EPPExists(EPPName));

    return EPP(EPPName).garrisonPriority();
}

int MachPhysEvasionPriorityPlans::aggressivesPriority(const std::string& EPPName) const
{
    PRE(EPPExists(EPPName));

    return EPP(EPPName).aggressivesPriority();
}

int MachPhysEvasionPriorityPlans::podPriority(const std::string& EPPName) const
{
    PRE(EPPExists(EPPName));

    return EPP(EPPName).podPriority();
}

int MachPhysEvasionPriorityPlans::turretsPriority(const std::string& EPPName) const
{
    PRE(EPPExists(EPPName));

    return EPP(EPPName).turretsPriority();
}

bool MachPhysEvasionPriorityPlans::EPPExists(const std::string& EPPName) const
{
    for (const std::unique_ptr<MachPhysEvasionPriorityPlan>& epp : EPPs_)
    {
        if (epp->name() == EPPName)
            return true;
    }

    return false;
}

const MachPhysEvasionPriorityPlan& MachPhysEvasionPriorityPlans::EPP(const std::string& EPPName) const
{
    PRE(EPPExists(EPPName));

    for (const std::unique_ptr<MachPhysEvasionPriorityPlan>& epp : EPPs_)
    {
        if (epp->name() == EPPName)
            return *epp;
    }

    ASSERT_FAIL("EPP not found");
    return *EPPs_.front();
}

MachPhysEvasionPriorityPlan& MachPhysEvasionPriorityPlans::EPP(const std::string& EPPName)
{
    PRE(EPPExists(EPPName));

    for (const std::unique_ptr<MachPhysEvasionPriorityPlan>& epp : EPPs_)
    {
        if (epp->name() == EPPName)
            return *epp;
    }

    ASSERT_FAIL("EPP not found");
    return *EPPs_.front();
}
