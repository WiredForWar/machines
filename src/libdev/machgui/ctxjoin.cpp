/*
 * C T X J O I N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/diag.hpp"
#include "machgui/ctxjoin.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/messbrok.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machgui/editlist.hpp"
#include "machgui/database.hpp"
#include "machgui/dbsystem.hpp"
#include "machgui/dbplanet.hpp"
#include "machgui/dbscenar.hpp"
#include "machlog/network.hpp"
#include "machgui/mdnetwork.hpp"
#include "machgui/mdserial.hpp"
#include "machgui/mdmodem.hpp"
#include "machgui/mdinternet.hpp"
#include "machgui/mdipx.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "gui/manager.hpp"
#include "machgui/internal/strings.hpp"
#include "network/netnet.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////

#define JOINGAME_LB_MINX 48
#define JOINGAME_LB_MAXX 262
#define JOINGAME_LB_MINY 56
#define JOINGAME_LB_MAXY 300
#define SCROLLBAR_WIDTH 17

MachGuiCtxJoin::MachGuiCtxJoin(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sc", pStartupScreens)
{
    WAYNE_STREAM("JOINGAME_LB_MAXY: " << JOINGAME_LB_MAXY << std::endl);
    pJoinBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 51, 605, 93) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_JOIN,
        MachGui::ButtonEvent::JOIN);
    pCreateBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 157, 605, 200) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CREATE,
        MachGui::ButtonEvent::CREATE);
    pShowGamesBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 251, 605, 293) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_SHOWGAMES,
        MachGui::ButtonEvent::SHOWGAMES);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 348, 605, 390) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);

    pCancelBtn->escapeControl(true);
    pShowGamesBtn_->defaultControl(true);

    // Display list box heading
    GuiResourceString currentGames(IDS_MENULB_GAMES);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    MachGuiMenuText* pCurrentGamesText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(JOINGAME_LB_MINX, JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
            Gui::Size(font.textWidth(currentGames.asString()), font.charHeight() + 1 * MachGui::menuScaleFactor())),
        IDS_MENULB_GAMES,
        MachGui::Menu::largeFontLight());

    // Create games list box
    pGamesList_ = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * JOINGAME_LB_MINX,
            pCurrentGamesText->absoluteBoundary().maxCorner().y() - getBackdropTopLeft().first,
            MachGui::menuScaleFactor() * (JOINGAME_LB_MAXX - SCROLLBAR_WIDTH),
            MachGui::menuScaleFactor() * JOINGAME_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);

    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(JOINGAME_LB_MAXX - SCROLLBAR_WIDTH, JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
        (JOINGAME_LB_MAXY - JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
        pGamesList_);

    pGamesList_->setSelectionChangedCallback([this](GuiSingleSelectionListBox* pBox) {
        ASSERT(pBox == pGamesList_, "");
        onGamesListSelectionChanged();
    });

    // Clear join game info.
    pStartupScreens->startupData()->resetData();
    addNewGameListItem();

    MachLogNetwork::instance().resetSession();
    // Query network to find which protocol has been selected
    NetNetwork::NetworkProtocol protocol = NetNetwork::instance().currentProtocol();

    switch (protocol)
    {
        case NetNetwork::NetworkProtocol::UDP:
            pNetworkProtocol_ = new MachGuiInternetNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::IPX:
            pNetworkProtocol_ = new MachGuiIPXNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::TCPIP:
            pNetworkProtocol_ = new MachGuiInternetNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::MODEM:
            pNetworkProtocol_ = new MachGuiModemNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::SERIAL:
            pNetworkProtocol_ = new MachGuiSerialNetworkMode(pStartupScreens, pStartupScreens);
            break;
        default:
            ASSERT(false, "Network protocol not currently supported by GUI");
    }

    TEST_INVARIANT;
}

MachGuiCtxJoin::~MachGuiCtxJoin()
{
    TEST_INVARIANT;

    GuiManager::instance().charFocus(pStartupScreens_);
}

void MachGuiCtxJoin::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxJoin& t)
{

    o << "MachGuiCtxJoin " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxJoin " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxJoin::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::SHOWGAMES)
    {
        showGames();
    }
}

void MachGuiCtxJoin::showGames()
{
    PRE(pNetworkProtocol_->connectionSetNoRecord());

    bool isHost = false;

    if (validNetworkSettings(isHost))
    {
        // Display PleaseWait
        // if ( NetNetwork::instance().currentProtocol() != NetNetwork::IPX )
        //{
        // GuiBitmap loadingBmp = Gui::bitmap( "gui/menu/pleaseWait.bmp" );
        // loadingBmp.enableColourKeying();
        // GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
        // frontBuffer.simpleBlit( loadingBmp, Ren::Rect( 0,0,loadingBmp.width(),loadingBmp.height() ) );
        //}

        // Clear games list box
        pGamesList_->deleteAllItems();
        addNewGameListItem();

        // Update network status
        NETWORK_STREAM("MachGuiCtxJoin::showGames() calling update to get most uptodate session list\n");
        NetNetwork::instance().refreshSessions();
    }
}

std::size_t MachGuiCtxJoin::numGamesInList() const
{
    // The list should always have at least the 'new game' item (which we exclude here)
    PRE(pGamesList_->numListItems() > 0);
    return pGamesList_->numListItems() - 1;
}

void MachGuiCtxJoin::addNewGameListItem()
{
    const int itemWidth = (JOINGAME_LB_MAXX - JOINGAME_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor();
    // Setup edit box for new game name to be entered into...
    GuiResourceString enterName(IDS_MENU_ENTERGAMENAME);
    pNewGameName_ = new MachGuiEditBoxListBoxItem(pStartupScreens_, pGamesList_, itemWidth, enterName.asString());
    pNewGameName_->maxChars(MAX_GAMENAME_LEN);
    pGamesList_->childrenUpdated();
}

void MachGuiCtxJoin::onGamesListSelectionChanged()
{
    if (pGamesList_->currentItem() == pNewGameName_)
    {
        pNewGameName_->setText(pStartupScreens_->startupData()->newGameName());
        editingGameName_ = true;
    }
    else
    {
        if (editingGameName_)
        {
            pStartupScreens_->startupData()->newGameName(pNewGameName_->text());

            changeFocus();
            editingGameName_ = false;
        }

        if (pGamesList_->currentItem())
        {
            std::optional<std::size_t> currentIndex = pGamesList_->getCurrentItemIndex();
            if (currentIndex.value_or(0) > 0)
            {
                std::size_t sessionIndex = currentIndex.value() - 1;
                const NetNetwork::Sessions& sessions = NetNetwork::instance().sessions();
                ASSERT(sessionIndex < sessions.size(), "Invalid game session list item");
                if (sessionIndex < sessions.size())
                {
                    const NetSessionInfo& info = sessions.at(sessionIndex);

                    NetNetwork::instance().setIPAddress(info.address);
                    pNetworkProtocol_->readNetworkDetails();
                }
            }
        }
    }

    // Update buttons
    bool newGameSelected = pGamesList_->currentItem() == pNewGameName_;
    bool anotherSessionSelected = !newGameSelected && pGamesList_->currentItem();

    pCreateBtn_->defaultControl(newGameSelected);
    pJoinBtn_->defaultControl(anotherSessionSelected);
    pShowGamesBtn_->defaultControl(!newGameSelected && !anotherSessionSelected);

    if(pCreateBtn_->hasFocusSet() || pJoinBtn_->hasFocusSet() || pShowGamesBtn_->hasFocusSet())
    {
        pCreateBtn_->hasFocus(pCreateBtn_->isDefaultControl());
        pJoinBtn_->hasFocus(pJoinBtn_->isDefaultControl());
        pShowGamesBtn_->hasFocus(pShowGamesBtn_->isDefaultControl());
    }
}

// virtual
bool MachGuiCtxJoin::okayToSwitchContext()
{
    // Initialise multiplayer settings ( assumes at least 1 planet and 1 scenario in Small map size ).
    MachGuiDbSystem& system = MachGuiDatabase::instance().multiPlayerSystem(MachGuiDatabase::SMALL);
    MachGuiDbPlanet& planet = system.planet(0);
    MachGuiDbScenario& scenario = planet.scenario(0);
    pStartupScreens_->startupData()->scenario(&scenario);
    // Initialise player slots to unavailable if scenario is limited in number of players.
    // Based on the maximum number of players allowed in this multiplayer game update the
    // player slots to "open" or "unavailable".
    for (int loop = 0; loop < 4; ++loop)
    {
        if (loop < scenario.maxPlayers())
        {
            pStartupScreens_->startupData()->players()[loop].setPlayerName(MachGui::openText());
        }
        else
        {
            pStartupScreens_->startupData()->players()[loop].setPlayerName(MachGui::unavailableText());
        }
    }

    // Was CREATE pressed...
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::CREATE)
    {
        bool isHost = true;

        if (pStartupScreens_->startupData()->newGameName() == "" || ! editingGameName_ || pNewGameName_->text().empty())
        {
            // Display message box. Type in game name to create game.
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERGAMENAME);
            return false;
        } // Can the network connection be established
        else if (! validNetworkSettings(isHost))
        {
            return false;
        }
        else
        {
            // Now pass through player name as input in gui to network library
            MachLogNetwork::instance().hostWithSessionId(
                pStartupScreens_->startupData()->newGameName(),
                pStartupScreens_->startupData()->playerName());

            if (NetNetwork::currentStatus() != NetNetwork::NETNET_OK)
            {
                pStartupScreens_->displayMsgBox(MachGui::convertNetworkError(NetNetwork::currentStatus()));
                NetNetwork::instance().resetStatus();
                return false;
            }

            pStartupScreens_->startupData()->resetData();
            pStartupScreens_->startupData()->hostGame(true);

            return true;
        }
    } // Was JOIN pressed...
    else if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::JOIN)
    {
        std::string ipAddress = NetNetwork::instance().IPAddress();
        bool isHost = false;
        if (ipAddress.empty())
        {
            // Display message box. Must choose game to join.
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_SELECTGAMETOJOIN);
            return false;
        } // Can the network connection be established
        // Complete rubbish! the connection settings have already been set. Hal 19/11/98
        //      else if ( not validNetworkSettings( isHost ) )
        //      {
        //          return false;
        //      }
        else
        {
            if (!MachLogNetwork::instance().joinSession(ipAddress, pStartupScreens_->startupData()->playerName()))
                if (!MachLogNetwork::instance().joinSession(ipAddress, pStartupScreens_->startupData()->playerName()))
                {
                    pStartupScreens_->displayMsgBox(IDS_MENUMSG_NETSESSIONERROR);
                    NetNetwork::instance().resetStatus();
                    return false;
                }

            if (NetNetwork::currentStatus() != NetNetwork::NETNET_OK)
            {
                pStartupScreens_->displayMsgBox(MachGui::convertNetworkError(NetNetwork::currentStatus()));
                NetNetwork::instance().resetStatus();
                return false;
            }
            // Reset join game incase player cancels "I'm Ready" context and wants to join a different game.
            pStartupScreens_->startupData()->resetData();
            pStartupScreens_->startupData()->hostGame(false);
            // Send join message so that host can update availablePlayers list etc.
            pStartupScreens_->messageBroker().sendJoinMessage(
                pStartupScreens_->startupData()->playerName(),
                pStartupScreens_->startupData()->uniqueMachineNumber());
            return true;
        }
    }
    return true;
}

// virtual
void MachGuiCtxJoin::update()
{
    // Get caret to flash in edit box.
    pNewGameName_->update();
    animations_.update();
    pNetworkProtocol_->updateGUI();
    NetNetwork::instance().update();

    const NetNetwork::Sessions& sessions = NetNetwork::instance().sessions();
    if (numGamesInList() != sessions.size())
    {
        const int itemWidth = (JOINGAME_LB_MAXX - JOINGAME_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor();
        for (std::size_t sessionIndex = numGamesInList(); sessionIndex < sessions.size(); ++sessionIndex)
        {
            new MachGuiSingleSelectionListBoxItem(
                pStartupScreens_,
                pGamesList_,
                itemWidth,
                sessions.at(sessionIndex).serverName);
            NETWORK_STREAM("  item added.\n");
        }
        pGamesList_->childrenUpdated();
    }
}

void MachGuiCtxJoin::changeFocus()
{
    pNetworkProtocol_->charFocus();
}

bool MachGuiCtxJoin::validNetworkSettings(bool isHost)
{
    // If network details are not valid, the class handling the protocol will display
    // the appropriate dialog box
    if (! pNetworkProtocol_->validNetworkDetails(isHost))
    {
        return false;
    }
    else
    {
        // Set network protocol settings to be the ones from the GUI
        pNetworkProtocol_->setNetworkDetails();
    }
    // Test for successful network connection
    if (! pNetworkProtocol_->connectionSet())
    {
        return false;
    }
    return true;
}
