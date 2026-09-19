/*
 * M L N O T I F . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogNotifiable

    Abstract Base Class to be inherited from by any class that wishes to be notified by
    a ConstructionTree object when it has changed.
*/

#ifndef _MACHLOG_MLNOTIF_HPP
#define _MACHLOG_MLNOTIF_HPP

#include "machphys/machphys.hpp"

class MachLogNotifiable
// Canonical form revoked
{
public:
    MachLogNotifiable(MachPhys::Race r);
    virtual ~MachLogNotifiable();

    virtual void notifiableBeNotified() = 0; // pure virtual

    void CLASS_INVARIANT;

    // The race whose tree changes this observer is notified about. Setting it
    // redirects later notifications and does nothing else: anything already
    // built from the previous race stays as it is until it is rebuilt.
    MachPhys::Race notifiedRace() const { return race_; }

    void setNotifiedRace(MachPhys::Race race);

private:
    MachLogNotifiable(const MachLogNotifiable&);
    MachLogNotifiable& operator=(const MachLogNotifiable&);
    bool operator==(const MachLogNotifiable&);

    MachPhys::Race race_;
};

#endif

/* End MLNOTIF.HPP *************************************************/
