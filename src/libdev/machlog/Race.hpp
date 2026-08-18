/*
 * R A C E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachRace

    The SimProcess representing a Machines race.
*/

#ifndef _RACE_HPP
#define _RACE_HPP

#include "base/base.hpp"
#include "sim/Process.hpp"
#include "machphys/machphys.hpp"

// Orthodox canonical (revoked)
class MachActor;

class MachLogRace : public SimProcess
{
public:
    MachLogRace(const MachPhys::Race& race);
    ~MachLogRace() override;

    const MachPhys::Race& race() const { return race_; };

    // True when this process simulates the race, rather than watching another host do it
    // and applying what that host reports.
    bool isSimulatedHere() const;

    void addActor(MachActor*);

    void CLASS_INVARIANT;

private:
    // Operations deliberately revoked
    MachLogRace(const MachLogRace&);
    MachLogRace& operator=(const MachLogRace&);
    bool operator==(const MachLogRace&);

    // Data members
    MachPhys::Race race_;
};

#endif

/* End RACE.HPP *****************************************************/
