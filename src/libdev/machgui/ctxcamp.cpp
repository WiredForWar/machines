/*
 * C T X C A M P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxcamp.hpp"

#include "base/diag.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/editlist.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/database.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/dbsavgam.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/msgbox.hpp"
#include "machgui/dbplyscn.hpp"
#include "machgui/dbscenar.hpp"
#include "system/registry.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "machgui/internal/strings.hpp"
#include "ctl/vector.hpp"

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiPlayerListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    MachGuiPlayerListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        MachGuiDbPlayer& player,
        MachGuiCtxCampaign* pCampaignCtx)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, player.name())
        , player_(player)
        , pCampaignCtx_(pCampaignCtx)
    {
        // Select this list box item if it was the last one selected
        if (player.name() == SysRegistry::instance().queryStringValue("Misc", "Current Player Name"))
        {
            selectThisItem();
        }
    }

    ~MachGuiPlayerListBoxItem() override { }

    void CLASS_INVARIANT;

protected:
    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();

        pCampaignCtx_->selectedPlayer(&player_);
        MachGuiDatabase::instance().currentPlayer(&player_);

        // Store the fact that this item was just selected
        SysRegistry::instance().setStringValue("Misc", "Current Player Name", player_.name());
    }

    void unselect() override
    {
        MachGuiSingleSelectionListBoxItem::unselect();

        pCampaignCtx_->clearSelectedPlayer();
    }

private:
    MachGuiPlayerListBoxItem(const MachGuiPlayerListBoxItem&);
    MachGuiPlayerListBoxItem& operator=(const MachGuiPlayerListBoxItem&);

    // Data members...
    MachGuiDbPlayer& player_;
    MachGuiCtxCampaign* pCampaignCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiPlayerDeleteMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiPlayerDeleteMessageBoxResponder(MachGuiCtxCampaign* pCampaignCtx)
        : pCampaignCtx_(pCampaignCtx)
    {
    }

    bool okPressed() override
    {
        pCampaignCtx_->deletePlayer();
        return true;
    }

private:
    MachGuiCtxCampaign* pCampaignCtx_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

MachGuiCtxCampaign::MachGuiCtxCampaign(MachGuiStartupScreens* pStartupScreens)
    : MachGuiStartupScreenContext(pStartupScreens)
    , animations_(pStartupScreens, SysPathName("gui/menu/sm_anims.anm"))
    , pSelectedPlayer_(nullptr)
    , pScenarioText_(nullptr)
    , pLastScenarioText_(nullptr)
    , pScoreText_(nullptr)
    , pLastScoreText_(nullptr)
    , pNumScenariosPlayedText_(nullptr)
    , pNumScenariosText_(nullptr)
    , pTotalScoreHeadingText_(nullptr)
    , pTotalScoreText_(nullptr)
{
    const int PLAYERS_LB_MINX = 84;
    const int PLAYERS_LB_MINY = 93;
    const int PLAYERS_LB_MAXX = 237;
    const int PLAYERS_LB_MAXY = 378;
    const int CURRENTSTATUS_MINX = 378;
    const int CURRENTSTATUS_MINY = 225;
    const int CURRENTSTATUS_MAXX = 532;
    const int CURRENTSTATUS_MAXY = 376;

    MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(83, 408, 255, 445),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::OK);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(327, 408, 497, 445),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(376, 40, 546, 77),
        IDS_MENUBTN_NEW,
        MachGui::ButtonEvent::NEW);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(376, 134, 546, 172),
        IDS_MENUBTN_DELETE,
        MachGui::ButtonEvent::DELETE);

    pCancelBtn->escapeControl(true);
    pOkBtn->defaultControl(true);

    // Display list box heading
    GuiResourceString players(IDS_MENULB_PLAYERS);
    GuiBmpFont font(GuiBmpFont::getFont("gui/menu/largefnt.bmp"));
    MachGuiMenuText* pPlayersText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            PLAYERS_LB_MINX,
            PLAYERS_LB_MINY,
            PLAYERS_LB_MINX + font.textWidth(players.asString()),
            PLAYERS_LB_MINY + font.charHeight() + 2),
        IDS_MENULB_PLAYERS,
        "gui/menu/largefnt.bmp");

    // Display current status heading
    GuiResourceString status(IDS_MENU_PLAYERSTATUS);
    MachGuiMenuText* pCurrentStatusHeading = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            CURRENTSTATUS_MINX,
            CURRENTSTATUS_MINY,
            CURRENTSTATUS_MINX + font.textWidth(status.asString()),
            CURRENTSTATUS_MINY + font.charHeight() + 2),
        IDS_MENU_PLAYERSTATUS,
        "gui/menu/largefnt.bmp");

    // Create players list box
    pPlayersList_ = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            PLAYERS_LB_MINX,
            pPlayersText->absoluteBoundary().maxCorner().y() - getBackdropTopLeft().first,
            PLAYERS_LB_MAXX,
            PLAYERS_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);
    updatePlayersList();

    // Display backdrop, play correct music, switch cursor on.
    changeBackdrop("gui/menu/sm.bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);

    TEST_INVARIANT;
}

MachGuiCtxCampaign::~MachGuiCtxCampaign()
{
    TEST_INVARIANT;
}

void MachGuiCtxCampaign::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxCampaign& t)
{

    o << "MachGuiCtxCampaign " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxCampaign " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxCampaign::update()
{
    animations_.update();
    pNewPlayerName_->update();
}

void MachGuiCtxCampaign::updatePlayersList()
{
    const int PLAYERS_LB_MINX = 84;
    const int PLAYERS_LB_MINY = 93;
    const int PLAYERS_LB_MAXX = 237;
    const int PLAYERS_LB_MAXY = 378;

    pPlayersList_->deleteAllItems();
    pSelectedPlayer_ = nullptr;

    // Create special [new player name] entry in list box.
    GuiResourceString newPlayerNameStr(IDS_MENU_NEWPLAYERNAME);
    pNewPlayerName_ = new MachGuiEditBoxListBoxItem(
        pStartupScreens_,
        pPlayersList_,
        PLAYERS_LB_MAXX - PLAYERS_LB_MINX,
        newPlayerNameStr.asString());
    pNewPlayerName_->maxChars(MAX_PLAYERNAME_LEN);

    // Add previously created players into list box.
    uint numPlayers = MachGuiDatabase::instance().nPlayers();

    for (uint loop = 0; loop < numPlayers; ++loop)
    {
        MachGuiDbPlayer& player = MachGuiDatabase::instance().player(loop);
        new 
            MachGuiPlayerListBoxItem(pStartupScreens_, pPlayersList_, PLAYERS_LB_MAXX - PLAYERS_LB_MINX, player, this);
    }

    // Get list to redraw.
    pPlayersList_->childrenUpdated();

    displayCurrentStatus();
}

// virtual
bool MachGuiCtxCampaign::okayToSwitchContext()
{
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::OK)
    {
        // Create new player...
        if (pNewPlayerName_->selected())
        {
            if (pNewPlayerName_->text() != "")
            {
                // Store the name of the last selected player in the registry
                SysRegistry::instance().setStringValue("Misc", "Current Player Name", pNewPlayerName_->text());

                MachGuiDbPlayer& newPlayer = MachGuiDatabase::instance().addPlayer(pNewPlayerName_->text());
                MachGuiDatabase::instance().currentPlayer(&newPlayer);
                MachGuiDatabase::instance().writeDatabase();

                return true;
            }
            else
            {
                pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERPLAYERNAME);
                return false;
            }
        }
        else // Select previously created player...
        {
            if (! pSelectedPlayer_)
            {
                pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERPLAYERNAME);
                return false;
            }
        }
    }
    else if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::EXIT)
    {
        MachGuiDatabase::instance().clearCurrentPlayer();
    }

    return true;
}

void MachGuiCtxCampaign::selectedPlayer(MachGuiDbPlayer* pPlayer)
{
    pSelectedPlayer_ = pPlayer;
    displayCurrentStatus();
}

void MachGuiCtxCampaign::clearSelectedPlayer()
{
    pSelectedPlayer_ = nullptr;
    displayCurrentStatus();
}

// virtual
void MachGuiCtxCampaign::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::NEW)
    {
        pNewPlayerName_->selectThisItem();
        pStartupScreens_->displayMsgBox(IDS_MENUMSG_NEWPLAYERNAME);
    }
    else if (be == MachGui::ButtonEvent::DELETE)
    {
        if (pSelectedPlayer_)
        {
            // Confirm deletion. Display players name in delete msg.
            GuiStrings strings;
            strings.push_back(pSelectedPlayer_->name());
            pStartupScreens_->displayMsgBox(
                IDS_MENUMSG_DELETEPLAYER,
                new MachGuiPlayerDeleteMessageBoxResponder(this)),
                strings;
        }
        else
        {
            if (pNewPlayerName_->selected())
            {
                // Reset new player name
                pNewPlayerName_->text("");
            }
        }
    }
}

void MachGuiCtxCampaign::deletePlayer()
{
    PRE(pSelectedPlayer_);

    // Delete player
    MachGuiDatabase::instance().removePlayer(pSelectedPlayer_);

    // Construct a list of saved games with no player attached
    typedef ctl_vector<MachGuiDbSavedGame*> SavedGames;
    SavedGames savedGamesWithNoPlayer;
    savedGamesWithNoPlayer.reserve(64);

    uint numSavedGames = MachGuiDatabase::instance().nSavedGames();

    for (uint loop = 0; loop < numSavedGames; ++loop)
    {
        MachGuiDbSavedGame& savedGame = MachGuiDatabase::instance().savedGame(loop);
        if (savedGame.isCampaignGame() && ! savedGame.hasPlayer())
            savedGamesWithNoPlayer.push_back(&savedGame);
    }

    // Now delete these saved games.
    // Note this 2-stage process is necessary, otherwise we will be modifying a collection
    // whilst iterating through it.
    for (uint loop = 0; loop < savedGamesWithNoPlayer.size(); ++loop)
    {
        MachGuiDbSavedGame* pSavedGame = savedGamesWithNoPlayer[loop];
        remove(pSavedGame->externalFileName().c_str());
        MachGuiDatabase::instance().removeSavedGame(pSavedGame);
    }

    // Update persistent version of database
    MachGuiDatabase::instance().writeDatabase();
    updatePlayersList();
}

void MachGuiCtxCampaign::displayCurrentStatus()
{
    const int CURRENTSTATUS_MINX = 378;
    const int CURRENTSTATUS_MINY = 260;
    const int CURRENTSTATUS_MAXX = 532;
    const int CURRENTSTATUS_MAXY = 376;
    const int CURRENTSTATUS_WIDTH = CURRENTSTATUS_MAXX - CURRENTSTATUS_MINX;

    // Clean up old text
    delete pScenarioText_;
    delete pLastScenarioText_;
    delete pScoreText_;
    delete pLastScoreText_;
    delete pNumScenariosPlayedText_;
    delete pNumScenariosText_;
    delete pTotalScoreHeadingText_;
    delete pTotalScoreText_;
    pScenarioText_ = nullptr;
    pLastScenarioText_ = nullptr;
    pScoreText_ = nullptr;
    pLastScoreText_ = nullptr;
    pNumScenariosPlayedText_ = nullptr;
    pNumScenariosText_ = nullptr;
    pTotalScoreHeadingText_ = nullptr;
    pTotalScoreText_ = nullptr;

    pStartupScreens_->changed();

    const auto& topLeft = getBackdropTopLeft();

    // Only display information if a player has been selected
    if (pSelectedPlayer_)
    {
        // Load resource strings
        GuiResourceString scenarioStr(IDS_MENU_STATUSSCENARIO);
        GuiResourceString scoreStr(IDS_MENU_STATUSSCORE);
        GuiResourceString numScenariosPlayedStr(IDS_MENU_STATUSNUMSCENARIOS);
        GuiResourceString totalScoreStr(IDS_MENU_STATUSTOTALSCORE);
        GuiResourceString lastScenarioGuiStr(IDS_MENU_STATUSNOTPLAYED);

        // Default text
        string lastScenarioStr(lastScenarioGuiStr.asString());
        string lastScoreStr("0");
        string numScenariosPlayed("0");
        string totalScore("0");

        if (pSelectedPlayer_->nPlayerScenarios() != 0)
        {
            MachGuiDbPlayerScenario& lastScenario = pSelectedPlayer_->mostRecentPlayerScenario();

            // Info about last scenario played
            lastScenarioStr = lastScenario.scenario().menuString();

            char buffer[255];

            //          lastScoreStr = itoa( lastScenario.lastScore(), buffer, 10 );
            sprintf(buffer, "%d", lastScenario.lastScore());
            lastScoreStr = buffer;
            //          numScenariosPlayed = itoa( pSelectedPlayer_->nPlayerScenarios(), buffer, 10 );
            sprintf(buffer, "%d", pSelectedPlayer_->nPlayerScenarios());
            numScenariosPlayed = buffer;

            // Total all the scores for scenarios played
            int nTotalScore = 0;
            for (int loop = 0; loop < pSelectedPlayer_->nPlayerScenarios(); ++loop)
            {
                nTotalScore += pSelectedPlayer_->playerScenario(loop).lastScore();
            }
            sprintf(buffer, "%d", nTotalScore);
            totalScore = buffer;
            //          totalScore = itoa( nTotalScore, buffer, 10 );
        }

        GuiBmpFont font(GuiBmpFont::getFont("gui/menu/smallfnt.bmp"));

        Gui::Box scenarioTextBox(
            Gui::Coord(CURRENTSTATUS_MINX, CURRENTSTATUS_MINY),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pScenarioText_ " << scenarioTextBox << std::endl);

        pScenarioText_ = new MachGuiMenuText(
            pStartupScreens_,
            scenarioTextBox,
            scenarioStr.asString(),
            "gui/menu/smallfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box lastScenarioTextBox(
            Gui::Coord(CURRENTSTATUS_MINX, pScenarioText_->absoluteBoundary().maxCorner().y() + 2 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            (2 * font.charHeight()) + 1);

        DEBUG_STREAM(DIAG_NEIL, "pLastScenarioText_ " << lastScenarioTextBox << std::endl);

        pLastScenarioText_ = new MachGuiMenuText(
            pStartupScreens_,
            lastScenarioTextBox,
            lastScenarioStr,
            "gui/menu/smalwfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box scoreTextBox(
            Gui::Coord(CURRENTSTATUS_MINX, pLastScenarioText_->absoluteBoundary().maxCorner().y() + 4 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pScoreText_ " << scoreTextBox << std::endl);

        pScoreText_ = new MachGuiMenuText(
            pStartupScreens_,
            scoreTextBox,
            scoreStr.asString(),
            "gui/menu/smallfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box lastScoreTextBox(
            Gui::Coord(CURRENTSTATUS_MINX, pScoreText_->absoluteBoundary().maxCorner().y() + 2 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pLastScoreText_ " << lastScoreTextBox << std::endl);

        pLastScoreText_ = new MachGuiMenuText(
            pStartupScreens_,
            lastScoreTextBox,
            lastScoreStr,
            "gui/menu/smalwfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box numScenariosPlayedStrBox(
            Gui::Coord(CURRENTSTATUS_MINX, pLastScoreText_->absoluteBoundary().maxCorner().y() + 6 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pNumScenariosPlayedText_ " << numScenariosPlayedStrBox << std::endl);

        pNumScenariosPlayedText_ = new MachGuiMenuText(
            pStartupScreens_,
            numScenariosPlayedStrBox,
            numScenariosPlayedStr.asString(),
            "gui/menu/smallfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box numScenariosPlayedBox(
            Gui::Coord(
                CURRENTSTATUS_MINX,
                pNumScenariosPlayedText_->absoluteBoundary().maxCorner().y() + 2 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pNumScenariosText_ " << numScenariosPlayedBox << std::endl);

        pNumScenariosText_ = new MachGuiMenuText(
            pStartupScreens_,
            numScenariosPlayedBox,
            numScenariosPlayed,
            "gui/menu/smalwfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box totalScoreStrBox(
            Gui::Coord(CURRENTSTATUS_MINX, pNumScenariosText_->absoluteBoundary().maxCorner().y() + 6 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pTotalScoreHeadingText_ " << totalScoreStrBox << std::endl);

        pTotalScoreHeadingText_ = new MachGuiMenuText(
            pStartupScreens_,
            totalScoreStrBox,
            totalScoreStr.asString(),
            "gui/menu/smallfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);

        Gui::Box totalScoreBox(
            Gui::Coord(
                CURRENTSTATUS_MINX,
                pTotalScoreHeadingText_->absoluteBoundary().maxCorner().y() + 2 - topLeft.first),
            CURRENTSTATUS_WIDTH,
            font.charHeight());

        DEBUG_STREAM(DIAG_NEIL, "pTotalScoreText_ " << totalScoreBox << std::endl);

        pTotalScoreText_ = new MachGuiMenuText(
            pStartupScreens_,
            totalScoreBox,
            totalScore,
            "gui/menu/smalwfnt.bmp",
            MachGuiMenuText::LEFT_JUSTIFY);
    }
}

/* End CTXCAMP.CPP **************************************************/
