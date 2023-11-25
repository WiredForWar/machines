/*
 * D I S P N O T I . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "gui/restring.hpp"
#include "gui/font.hpp"
#include "machgui/dispnoti.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/chatmsgs.hpp"
#include "machlog/races.hpp"
#include "machlog/cntrl_pc.hpp"
#include "machgui/internal/strings.hpp"

MachGuiDispositionChangeNotifiable::MachGuiDispositionChangeNotifiable(MachGuiStartupScreens* pStartupScreens)
    : pStartupScreens_(pStartupScreens)
{

    TEST_INVARIANT;
}

MachGuiDispositionChangeNotifiable::~MachGuiDispositionChangeNotifiable()
{
    TEST_INVARIANT;
}

void MachGuiDispositionChangeNotifiable::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDispositionChangeNotifiable& t)
{

    o << "MachGuiDispositionChangeNotifiable " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDispositionChangeNotifiable " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiDispositionChangeNotifiable::notifyGeneralDispositionChange()
{
}

// virtual
void MachGuiDispositionChangeNotifiable::notifySpecificDispositionChange(MachPhys::Race, MachPhys::Race)
{
}

// virtual
void MachGuiDispositionChangeNotifiable::notifyDispositionChangeToNoneAlly(
    MachPhys::Race checkRace,
    MachPhys::Race targetRace)
{
    doDisplay(IDS_DISPOSITION_TO_NONE_ALLY, checkRace, targetRace);
}

// virtual
void MachGuiDispositionChangeNotifiable::notifyDispositionChangeToAlly(
    MachPhys::Race checkRace,
    MachPhys::Race targetRace)
{
    doDisplay(IDS_DISPOSITION_TO_ALLY, checkRace, targetRace);
}

void MachGuiDispositionChangeNotifiable::doDisplay(GuiResourceString::Id id, MachPhys::Race race1, MachPhys::Race race2)
{
    if (pStartupScreens_->gameType() == MachGuiStartupScreens::MULTIGAME)
    {
        MachPhys::Race pcsRace = MachLogRaces::instance().pcController().race();

        bool bothRacesAreAI = MachLogRaces::instance().controller(race1).type() == MachLogController::AI_CONTROLLER
            && MachLogRaces::instance().controller(race2).type() == MachLogController::AI_CONTROLLER;

        if (pStartupScreens_->startupData()->broadcastAlliances() || pcsRace == race1 || pcsRace == race2)
        {
            if (! bothRacesAreAI) // Never show computer to computer alliances
            {
                GuiStrings strings;
                strings.push_back(getDisplayName(race1));
                strings.push_back(getDisplayName(race2));

                GuiResourceString dispositionString(id, strings);

                MachGuiInGameChatMessages::instance().addMessage(dispositionString.asString());
            }
        }
    }
}

string MachGuiDispositionChangeNotifiable::getDisplayName(MachPhys::Race race)
{
    MachGuiStartupData::PlayerInfo* playerInfo = pStartupScreens_->startupData()->players();
    string name;
    // find a match with race
    switch (race)
    {
        case MachPhys::RED:
            name.assign(1, GuiBmpFont::redCharIndex());
            break;
        case MachPhys::GREEN:
            name.assign(1, GuiBmpFont::greenCharIndex());
            break;
        case MachPhys::BLUE:
            name.assign(1, GuiBmpFont::blueCharIndex());
            break;
        case MachPhys::YELLOW:
            name.assign(1, GuiBmpFont::yellowCharIndex());
            break;

        case MachPhys::N_RACES:
        case MachPhys::NORACE:
            break;
    }

    for (int i = 0; i < 4; ++i)
        if (playerInfo[i].race_ == race)
        {
            if (playerInfo[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN
                || playerInfo[i].status_ == MachGuiStartupData::PlayerInfo::CONNECTION_LOST)
            {
                name += playerInfo[i].playerName_;
            }
            else
            {
                name += GuiResourceString(IDS_MENU_STSCOMPUTER).asString();
            }
        }
    return name;
}

/* End DISPNOTI.CPP *************************************************/
