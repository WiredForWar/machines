/*
 * S Y S M E S S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#if defined PRODUCTION && defined DEMO
#undef PRODUCTION
#endif

#include "ctl/list.hpp"

#include "mathex/point2d.hpp"

#include "sim/process.hpp"
#include "sim/disevent.hpp"
#include "sim/manager.hpp"

#include "gui/restring.hpp"

#include "machlog/network.hpp"
#include "machlog/races.hpp"
#include "machlog/race.hpp"
#include "machlog/cntrl.hpp"
#include "machlog/actor.hpp"

#include "machgui/internal/strings.hpp"
#include "machgui/sysmess.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/chatmsgs.hpp"

class MachGuiSessionLostGameTerminatorEvent : public SimDiscreteEvent
{
public:
    MachGuiSessionLostGameTerminatorEvent(MachGuiStartupScreens* pStartupScreens, const PhysAbsoluteTime& time)
        : SimDiscreteEvent(time)
        , pStartupScreens_(pStartupScreens) {};
    void execute() override
    {
        NETWORK_STREAM("MachGuiSessionLostGameTerminatorEvent::execute\n");

        // if we got into this screen via a lobby session then we need to terminate correctly at this point.
        NETWORK_STREAM("MachGuiSessionLostGameTerminatorEvent::execute terminate and reset\n");
        MachLogNetwork::instance().terminateAndReset();
        NETWORK_STREAM("MachGuiSessionLostGameTerminatorEvent::execute contextFinish\n");
        // Changed
        pStartupScreens_->deleteAllChildren();
        //      pStartupScreens_->contextFinishFromLobby();
        //      NETWORK_STREAM("MachGuiSessionLostGameTerminatorEvent::execute DONE\n" );
    };

private:
    MachGuiStartupScreens* pStartupScreens_;
};

MachGuiSystemMessageHandler::MachGuiSystemMessageHandler(MachGuiStartupScreens* pStartupScreens)
    : pStartupScreens_(pStartupScreens)
{

    TEST_INVARIANT;
}

MachGuiSystemMessageHandler::~MachGuiSystemMessageHandler()
{
    TEST_INVARIANT;
}

void MachGuiSystemMessageHandler::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiSystemMessageHandler& t)
{

    o << "MachGuiSystemMessageHandler " << (void*)&t << " start" << std::endl;
    o << "MachGuiSystemMessageHandler " << (void*)&t << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiSystemMessageHandler::handleHostMessage()
{
    NETWORK_STREAM("MachGuiSystemMessageHandler::handleHostMessage\n");
    if (respondAsInGame())
    {
        NETWORK_STREAM("MachGuiStartupScreens::CTX_MULTI_GAME\n");
        MachLogNetwork::instance().isNodeLogicalHost(true);
        MachLogRaces& races = MachLogRaces::instance();
        for (MachPhys::Race i : MachPhys::AllRaces)
        {
            if (races.raceInGame(i) and races.controller(i).type() == MachLogController::AI_CONTROLLER)
            {
                MexPoint2d pos;
                if (races.aggressorAssemblyPoints(i).size() == 0 or races.aggressorAssemblyPoints(i).size() == 0)
                {
                    if (races.raceObjects(i).size() > 0)
                        pos = races.raceObjects(i).front()->position();
                    else
                        pos = MexPoint2d(400, 400);
                }
                races.addAggressorAssemblyPoint(i, pos);
                races.addAdministratorAssemblyPoint(i, pos);
                races.race(i).toBeUpdated(SimProcess::MANAGER_UPDATE);
                MachLogNetwork::instance().remoteStatus(i, MachLogNetwork::LOCAL_PROCESS);
            }
        }
        GuiResourceString errorString(
            IDS_MULTI_ERROR_HOST_CONNECTION_LOST,
            pStartupScreens_->startupData()->playerName());
        MachGuiInGameChatMessages::instance().addMessage(errorString.asString());
        return true;
        //      return MachLogSystemMessageHandler::handleHostMessage();
    }
    NETWORK_STREAM(" calling startupscreens handle host\n");
    return pStartupScreens_->handleHostMessage();
}

// virtual
bool MachGuiSystemMessageHandler::handleDestroyPlayerMessage(const string& name)
{
    NETWORK_STREAM("MachGuiSystemMessageHandler::handleDestroyPlayerMessage " << name << std::endl);
    NETWORK_INDENT(2);
    if (respondAsInGame())
    {
        NETWORK_STREAM("MachGuiStartupScreens::CTX_MULTI_GAME\n");
        MachPhys::Race r = MachPhys::N_RACES;
        bool foundPlayer = false;
        for (int i = 0; i < 4; ++i)
        {
            const string& playerName = pStartupScreens_->startupData()->players()[i].getDisplayName();
            NETWORK_STREAM("Checking against " << playerName << std::endl);
            if (playerName == name
                and pStartupScreens_->startupData()->players()[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN)
            {
                foundPlayer = true;
                NETWORK_STREAM("found a match with.\n");

                if (pStartupScreens_->startupData()->players()[i].host_)
                {
                    pStartupScreens_->startupData()->players()[i].host_ = false;
                    //                  //determine which player is next in list of races to become the host.
                    //                  int lowest = 4;
                    //                  for( int j = 3; j >= 0; --j )
                    //                      if( pStartupScreens_->startupData()->players()[i].status_ ==
                    //                      MachGuiStartupData::PlayerInfo::HUMAN and i != j )
                    //                          lowest = j;
                    //                  if( lowest == (int)pStartupScreens_->startupData()->gameSettings()->playerRace_
                    //                  )
                    //                      MachLogNetwork::instance().isNodeLogicalHost( true );
                    //                  GuiResourceString errorString( IDS_MULTI_ERROR_HOST_CONNECTION_LOST,
                    //                  pStartupScreens_->startupData()->players()[i].getDisplayName() );
                }

                r = pStartupScreens_->startupData()->players()[i].race_;
                pStartupScreens_->startupData()->players()[i].status_ = MachGuiStartupData::PlayerInfo::CONNECTION_LOST;
                GuiResourceString errorString(IDS_MULTI_ERROR_PLAYER_CONNECTION_LOST, name);
                MachGuiInGameChatMessages::instance().addMessage(errorString.asString());
                logicalHandleDestroyPlayerMessage(r);
            }
        }
        return true;
    }

    NETWORK_STREAM(" calling startupscreens handle destroy\n");
    return pStartupScreens_->handleDestroyPlayerMessage(name);
}

// virtual
bool MachGuiSystemMessageHandler::handleSessionLostMessage()
{
    NETWORK_STREAM("MachGuiSystemMessageHandler::handleSessionLostMessage\n");
    if (respondAsInGame())
    {
        GuiResourceString errorString(IDS_MULTI_ERROR_SESSION_LOST_INGAME);
        MachGuiInGameChatMessages::instance().addMessage(errorString.asString());
        MachGuiSessionLostGameTerminatorEvent* pEvent
            = new MachGuiSessionLostGameTerminatorEvent(pStartupScreens_, SimManager::instance().currentTime() + 5);
        SimManager::instance().add(pEvent);
        return true;
    }
    return pStartupScreens_->handleSessionLostMessage();
}

bool MachGuiSystemMessageHandler::respondAsInGame() const
{
    bool result = false;
    MachGuiStartupScreens::Context context = pStartupScreens_->currentContext();
    switch (context)
    {
        case MachGuiStartupScreens::CTX_MULTI_GAME:
        case MachGuiStartupScreens::CTX_INGAMEOP:
        case MachGuiStartupScreens::CTX_IGOPTIONS:
        case MachGuiStartupScreens::CTX_HOTKEYS:
        case MachGuiStartupScreens::CTX_MPINGAMEOPTIONS:
            result = true;
            break;
        default:
            result = false;
    }
    return result;
}

/* End SYSMESS.CPP **************************************************/
