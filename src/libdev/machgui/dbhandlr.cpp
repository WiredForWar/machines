/*
 * D B H A N D L R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dbhandlr.hpp"

#include "ctl/vector.hpp"
#include "machgui/database.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/dbplyscn.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/dbtexdat.hpp"

MachGuiDatabaseHandler::MachGuiDatabaseHandler()
    : MachLogDatabaseHandler()
{

    TEST_INVARIANT;
}

MachGuiDatabaseHandler::~MachGuiDatabaseHandler()
{
    TEST_INVARIANT;
}

void MachGuiDatabaseHandler::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDatabaseHandler& t)
{

    o << "MachGuiDatabaseHandler " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDatabaseHandler " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
const MachLogDatabaseHandler::Units&
MachGuiDatabaseHandler::survivingUnits(MachPhys::Race race, const std::string& scenarioName) const
{
    // Use an empty static collection to return if we don't find what we are looking for below
    static MachLogDatabaseHandler::Units dummyUnits;
    const MachLogDatabaseHandler::Units* pUnits = &dummyUnits;

    // Get the current player
    MachGuiDbPlayer& player = MachGuiDatabase::instance().currentPlayer();

    // Find the playerscenario object
    MachGuiDbScenario* pDbScenario = nullptr;
    if (MachGuiDatabase::instance().campaignScenario(scenarioName, &pDbScenario))
    {
        // See if our player has played this scenario successfully
        MachGuiDbPlayerScenario* pDbPlayerScenario = nullptr;
        if (player.hasPlayed(*pDbScenario, &pDbPlayerScenario) && pDbPlayerScenario->hasWon())
        {
            pUnits = &pDbPlayerScenario->raceSurvivingUnits(race);
        }
    }

    return *pUnits;
}

// virtual
bool MachGuiDatabaseHandler::isFlagSet(const std::string& flag, const std::string& scenarioName) const
{
    bool isSet = false;

    // Get the current player
    MachGuiDbPlayer& player = MachGuiDatabase::instance().currentPlayer();

    // Find the playerscenario object
    MachGuiDbScenario* pDbScenario = nullptr;
    if (MachGuiDatabase::instance().campaignScenario(scenarioName, &pDbScenario))
    {
        // See if our player has played this scenario successfully
        MachGuiDbPlayerScenario* pDbPlayerScenario = nullptr;
        if (player.hasPlayed(*pDbScenario, &pDbPlayerScenario) && pDbPlayerScenario->hasWon())
        {
            isSet = pDbPlayerScenario->isFlagSet(flag);
        }
    }

    return isSet;
}

// virtual
uint MachGuiDatabaseHandler::nTasksInCurrentScenario() const
{
    return MachGuiDatabase::instance().currentScenario().textData().nTasks();
}

// virtual
bool MachGuiDatabaseHandler::taskStartsAvailable(uint index) const
{
    return MachGuiDatabase::instance().currentScenario().textData().taskStartsAvailable(index);
}

/* End DBHANDLR.CPP *************************************************/
