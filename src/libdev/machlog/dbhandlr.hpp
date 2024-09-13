/*
 * D B H A N D L R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogDatabaseHandler

    An ABC that allows MachLog to interface with the game database. Concrete class to be
    defined in MachGui.
*/

#ifndef _MACHLOG_DBHANDLR_HPP
#define _MACHLOG_DBHANDLR_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "machphys/machphys.hpp"
#include "machlog/produnit.hpp" //Cheap, and saves clients with ctl_vector defined including it

#include <string>

template <class T> class ctl_vector;
class MachLogDatabaseHandlerImpl;

class MachLogDatabaseHandler
// Canonical form revoked
{
public:
    ~MachLogDatabaseHandler();

    //////////////////////////////////////////

    // Updates pUnits to include a production unit for each machine currently in
    // the specified race.
    using Units = ctl_vector<MachLogProductionUnit>;
    void raceUnits(MachPhys::Race race, Units* pUnits) const;

    // Get the surviving units of specified race at end of the named scenario
    virtual const Units& survivingUnits(MachPhys::Race race, const std::string& scenarioName) const = 0;

    //////////////////////////////////////////

    // Store the named flag as having been set
    void setScenarioFlag(const std::string& flag);

    // The number of set flags
    uint nSetFlags() const;

    // The index'th set flag
    const std::string& setFlag(uint index) const;
    // PRE( index < nSetFlags() );

    // Empty the list of set flags
    void clearSetFlags();

    // true if the named flag was set by the current player during the named scenario
    virtual bool isFlagSet(const std::string& flag, const std::string& scenarioName) const = 0;

    //////////////////////////////////////////

    // The number of sub-tasks if any defined in the current scenario
    virtual uint nTasksInCurrentScenario() const = 0;

    // True iff the index'th task of the current task is available from the start
    virtual bool taskStartsAvailable(uint index) const = 0;

    // Set the availablity of the index'th task in the current scenario
    void taskIsAvailable(uint index, bool isNow);
    // PRE( index < nTasksInCurrentScenario() );

    // True if the index'th task in the current scenario is available
    bool taskIsAvailable(uint index) const;
    // PRE( index < nTasksInCurrentScenario() );

    // Set the completion status of the index'th task in the current scenario
    void taskIsComplete(uint index, bool isNow);
    // PRE( index < nTasksInCurrentScenario() );

    // True if the index'th task in the current scenario is complete
    bool taskIsComplete(uint index) const;
    // PRE( index < nTasksInCurrentScenario() );

    // Called at start of each scenario to initialise the task flags
    void initialiseTaskFlags();

    // Clears all the task flags info
    void clearTaskFlags();

    //////////////////////////////////////////

    // write/read info stored for curent scenario as raw data
    void writePersistenceData(PerOstream& ostr);
    void readPersistenceData(PerIstream& istr);

    void CLASS_INVARIANT;

protected:
    MachLogDatabaseHandler();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogDatabaseHandler& t);

    // revoked
    MachLogDatabaseHandler(const MachLogDatabaseHandler&);
    MachLogDatabaseHandler& operator=(const MachLogDatabaseHandler&);

    // data members
    MachLogDatabaseHandlerImpl* pImpl_;
};

#endif

/* End DBHANDLR.HPP *************************************************/
