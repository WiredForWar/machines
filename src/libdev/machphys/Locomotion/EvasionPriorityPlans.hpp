/*
 * E P P s . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
 * MachPhysEvasionPriorityPlans is a collection of EPPs. An MachPhysEvasionPriorityPlan is an evasion priority
 *  plan, which designates the preference of evasion strategy a machine with that plan
 *  will look for in order, e.g. attempts to find nearby garrisons to hide in first, if
 *  that fails, tries to find nearby turrets within whose weapon umbrella to hide etc.
 *  It is designed as a singleton class.
 */

// canonical form revoked

#ifndef _MACHPHYS_EPPs_HPP
#define _MACHPHYS_EPPs_HPP

#include "base/base.hpp"

#include <memory>
#include <string>
#include <vector>

class MachPhysEvasionPriorityPlan;

class MachPhysEvasionPriorityPlans
{
public:
    enum
    {
        N_PRIORITY_LEVELS = 4,
    };

    static MachPhysEvasionPriorityPlans& instance();
    ~MachPhysEvasionPriorityPlans() = default;

    void clear();

    void addNewEPP(const std::string& EPPName);

    void garrisonPriority(const std::string& EPPName, int priority);
    void aggressivesPriority(const std::string& EPPName, int priority);
    void podPriority(const std::string& EPPName, int priority);
    void turretsPriority(const std::string& EPPName, int priority);

    int garrisonPriority(const std::string& EPPName) const;
    int aggressivesPriority(const std::string& EPPName) const;
    int podPriority(const std::string& EPPName) const;
    int turretsPriority(const std::string& EPPName) const;

    const MachPhysEvasionPriorityPlan& EPP(const std::string& EPPName) const;

    bool EPPExists(const std::string& EPPName) const;

private:
    MachPhysEvasionPriorityPlans() = default;
    MachPhysEvasionPriorityPlans(const MachPhysEvasionPriorityPlans&) = delete;
    MachPhysEvasionPriorityPlans& operator=(const MachPhysEvasionPriorityPlans&) = delete;

    MachPhysEvasionPriorityPlan& EPP(const std::string& EPPName);

    std::vector<std::unique_ptr<MachPhysEvasionPriorityPlan>> EPPs_;
};

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACHPHYS_EPPs_HPP  */

/* End EPPs.HPP *****************************************************/
