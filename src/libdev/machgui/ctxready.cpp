/*
 * C T X R E A D Y . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxready.hpp"

#include "base/diag.hpp"
#include "machgui/chatwind.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/messbrok.hpp"
#include "machgui/editbox.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/gui.hpp"
#include "machgui/playerit.hpp"
#include "machgui/dbplanet.hpp"
#include "machgui/dbsystem.hpp"
#include "machgui/msgboxei.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/Text.hpp"
#include "machlog/network.hpp"
#include "gui/manager.hpp"
#include "gui/event.hpp"
#include "gui/scrolist.hpp"
#include "gui/painter.hpp"
#include "gui/restring.hpp"
#include "device/keyboard.hpp"
#include "device/timer.hpp"
#include "afx/resource.hpp"
#include "network/netnet.hpp"
#include "network/session.hpp"
#include "network/nodeuid.hpp"
#include "network/node.hpp"
#include "system/winapi.hpp"
#include "machgui/menus_helper.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiImReadyButton : public MachGuiMenuButton
{
public:
    MachGuiImReadyButton(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        unsigned int stringId,
        MachGui::ButtonEvent buttonEvent)
        : MachGuiMenuButton(pParent, pParent, box, stringId, buttonEvent)
    {
        ready_ = false;
    }

    void setReady(bool ready)
    {
        if (ready_ != ready)
        {
            ready_ = ready;
            changed();
        }
    }

    void doDisplay() override
    {
        // Get base class to do majority of work
        MachGuiMenuButton::doDisplay();

        if (ready_)
        {
            GuiResourceString resString(stringId());
            string text = resString.asString();

            GuiBmpFont font(GuiBmpFont::getFont("gui/menu/largefnt.bmp"));
            size_t textWidth = font.textWidth(text);
            size_t textHeight = font.charHeight();

            size_t textX = absoluteBoundary().minCorner().x() + (width() - textWidth) / 2.0;
            size_t textY = absoluteBoundary().minCorner().y() + (height() - textHeight) / 2.0;

            textX -= MachGui::largeTickBmp().width() + 1;

            if (highlighted())
            {
                GuiPainter::instance().blit(MachGui::largeDarkTickBmp(), Gui::Coord(textX, textY));
            }
            else
            {
                GuiPainter::instance().blit(MachGui::largeTickBmp(), Gui::Coord(textX, textY));
            }
        }
    }

private:
    bool ready_;
};

////////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiChatButton : public MachGuiMenuButton
{
public:
    MachGuiChatButton(MachGuiStartupScreens* pParent, const Gui::Box& box)
        : MachGuiMenuButton(pParent, pParent, box, IDS_MENUBTN_SENDCHAT, MachGui::ButtonEvent::OK)
        , pStartupScreens_(pParent)
    {
    }

protected:
    void doHandleMouseClickEvent(const GuiMouseEvent&) override
    {
        // Do nothing
    }

    void doHandleMouseExitEvent(const GuiMouseEvent&) override
    {
        // Do nothing
    }

    void doHandleMouseEnterEvent(const GuiMouseEvent&) override
    {
        // Do nothing
    }

    bool executeControl() override
    {
        // Initiate button action
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::OK, "gui/sounds/chatmsg.wav");

        return true;
    }

private:
    MachGuiStartupScreens* pStartupScreens_;
};

#define SETTINGS_MINX 388
#define SETTINGS_MAXX 578
#define SETTINGS_MINY 110
#define SETTINGS_MAXY 270

MachGuiCtxImReady::MachGuiCtxImReady(MachGuiStartupScreens* pStartupScreens)
    : MachGuiStartupScreenContext(pStartupScreens)
    , animations_(pStartupScreens, SysPathName("gui/menu/sd_anims.anm"))
    , pCreationTimer_(new DevTimer())
    , pHasCDTransmissionTimer_(new DevTimer())
{
    NETWORK_STREAM("MachGuiCtxImReady::MachGuiCtxImReady this " << static_cast<const void*>(this) << "\n");
    NETWORK_INDENT(2);
    // Display backdrop, play correct music, switch cursor on.
    changeBackdrop("gui/menu/sd.bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);

    // Regular menu buttons...
    new MachGuiChatButton(pStartupScreens, Gui::Box(336, 345, 346, 373));

    MachGuiMenuButton* pCancelBtn;
    if (! NetNetwork::instance().isLobbiedGame())
        pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(379, 416, 574, 458),
            IDS_MENUBTN_CANCEL,
            MachGui::ButtonEvent::EXIT);
    else
        pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(379, 416, 574, 458),
            IDS_MENUBTN_EXIT_TO_ZONE,
            MachGui::ButtonEvent::EXIT);

    pSettingsButton_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(395, 40, 591, 82),
        IDS_MENUBTN_SETTINGS,
        MachGui::ButtonEvent::SETTINGS);
    pImReadyButton_ = new MachGuiImReadyButton(
        pStartupScreens,
        Gui::Box(47, 399, 243, 442),
        IDS_MENUBTN_IMREADY,
        MachGui::ButtonEvent::IMREADY);
    pStartButton_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(380, 348, 576, 392),
        IDS_MENUBTN_START,
        MachGui::ButtonEvent::START);
    pCancelBtn->escapeControl(true);

    // Chat window...
    pChatWindow_ = new MachGuiChatWindow(pStartupScreens, pStartupScreens, Gui::Box(30, 113, 345, 322));

    GuiBmpFont font(GuiBmpFont::getFont("gui/menu/smallfnt.bmp"));

    pSingleLineEditBox_
        = new MachGuiSingleLineEditBox(pStartupScreens, Gui::Box(37, 353, 330, 353 + font.charHeight()), font);
    pSingleLineEditBox_->maxChars(MAX_CHATMESSAGE_LEN - MAX_PLAYERNAME_LEN - 5);
    GuiResourceString chatMessage(IDS_MENU_CHATMESSAGE);
    pSingleLineEditBox_->text(chatMessage.asString());
    pSingleLineEditBox_->clearTextOnNextChar(true);
    GuiManager::instance().charFocus(pSingleLineEditBox_);

    pPlayerList_ = new GuiSimpleScrollableList(
        pStartupScreens,
        Gui::Box(76, 35, 316, 99),
        1000,
        MachGuiPlayerListItem::reqHeight(),
        1);

    createPlayerList();

    startupData().setCtxImReady(this);

    pReadOnlySettings_ = new GuiSimpleScrollableList(
        pStartupScreens,
        Gui::Box(SETTINGS_MINX, SETTINGS_MINY, SETTINGS_MAXX, SETTINGS_MAXY),
        (SETTINGS_MAXX - SETTINGS_MINX) * 0.66,
        MachGuiText::reqHeight() + 3,
        1);

    updateGameSettings();
    NetNetwork::instance().deterministicPingDropoutAllowed(true);

    int actualPlayers = 0;
    for (int i = 0; i < pStartupScreens->startupData()->gameSettings()->numPlayers_; ++i)
    {
        MachGuiStartupData::PlayerInfo::Status status = pStartupScreens->startupData()->players()[i].status_;
        if (status == MachGuiStartupData::PlayerInfo::HUMAN || status == MachGuiStartupData::PlayerInfo::COMPUTER)
            actualPlayers++;
    }

    TEST_INVARIANT;
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiCtxImReady::MachGuiCtxImReady DONE this " << static_cast<const void*>(this) << "\n");
}

MachGuiCtxImReady::~MachGuiCtxImReady()
{
    delete pCreationTimer_;
    delete pHasCDTransmissionTimer_;
    NETWORK_STREAM("MachGuiCtxImReady:: ~MachGuiCtxImReady this " << static_cast<const void*>(this) << "\n");
    NETWORK_INDENT(2);
    TEST_INVARIANT;

    startupData().removeCtxImReady();
    GuiManager::instance().charFocus(pStartupScreens_);
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiCtxImReady:: ~MachGuiCtxImReady DONE this " << static_cast<const void*>(this) << "\n");
}

void MachGuiCtxImReady::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxImReady& t)
{

    o << "MachGuiCtxImReady " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxImReady " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiCtxImReady::doHandleKeyEvent(const GuiKeyEvent& e)
{
    if (e.key() == DevKey::ENTER && e.state() == Gui::PRESSED)
    {
        // Make sure there is a chat message worth sending ( i.e. not empty )
        if (pSingleLineEditBox_->text() != "" && ! pSingleLineEditBox_->clearTextOnNextChar())
        {
            // Add players name onto chat message
            string chatMessage(startupData().playerName());
            chatMessage += " : ";
            chatMessage += pSingleLineEditBox_->text();

            // Send message (if host hasn't terminated or another problem hasn't occured )
            if (! pStartupScreens_->startupData()->terminalMultiPlayerGameProblem())
            {
                pStartupScreens_->messageBroker().sendChatMessage(chatMessage);
            }

            // I won't get notified of my own chat message therefore I must put it
            // in the chat window manually
            pChatWindow_->addText(chatMessage);

            // Clear down chat message entry area ready for next message
            pSingleLineEditBox_->text("");
        }

        return true;
    }

    return false;
}

// virtual
void MachGuiCtxImReady::update()
{
    pSingleLineEditBox_->update();
    animations_.update();

    if (MachLogNetwork::instance().isNetworkGame() && ! startupData().terminalMultiPlayerGameProblem())
    {
        /*      if( MachLogNetwork::instance().node().lastPingAllTime() > 2.5 )
        {
            // Send info to host to say if machines cd is present in this PC
            const double timeGap = ( NetNetwork::instance().currentProtocol() == NetNetwork::IPX ) ? 1.0 : 5.0;
            if ( not startupData().isHost() and not NetNetwork::instance().imStuffed() and
        pHasCDTransmissionTimer_->time() > timeGap )
            {
                pStartupScreens_->messageBroker().sendHasMachinesCDMessage( startupData().playerName(),
                                                                            MachGui::machinesCDIsAvailable( 1 ) or
        MachGui::machinesCDIsAvailable( 2 ) ); pHasCDTransmissionTimer_->time( 0 );
            }

            if( not NetNetwork::instance().imStuffed() )
                MachLogNetwork::instance().node().pingAll();
            //update pings every 1 second or so.
            NETWORK_STREAM("MachGuiCtxImReady::update updatePlayerList from back of pingAll this " << static_cast<const void*>(this) <<
        "\n" ); updatePlayerList();
        //  if( NetNetwork::instance().isLobbiedGame() )
                {
                    NETWORK_STREAM("MachGuiCtxImReady::MachGuiCtxImReady numPlayers_ " <<
        startupData().gameSettings()->numPlayers_ << std::endl ); for( int i = 0; i < 4; ++i )
                    {
                        MachGuiStartupData::PlayerInfo::Status status = startupData().players()[i].status_;
                        NETWORK_STREAM( startupData().players()[i].getDisplayName() << " " << (int)status << std::endl
        );
                    }
                    int actualPlayers = 0;
                    for( int i = 0; i < startupData().gameSettings()->numPlayers_; ++i )
                    {
                        MachGuiStartupData::PlayerInfo::Status status = startupData().players()[i].status_;
                        if( status == MachGuiStartupData::PlayerInfo::HUMAN or
                            status == MachGuiStartupData::PlayerInfo::COMPUTER )
                                actualPlayers++;
                    }
                    if( actualPlayers == 0 and pCreationTimer_->time() > 45 )
                    {
                        GuiResourceString errorNoPlayers( IDS_LOBBY_ERROR_NO_PLAYERS );
                        pChatWindow_->addText( errorNoPlayers.asString() );
                    }
                }
        }*/
    }
}

void MachGuiCtxImReady::createPlayerList()
{
    NETWORK_STREAM("MachGuiCtxImReady::createPlayerList\n");
    NETWORK_INDENT(2);
    MachGuiStartupData::PlayerInfo* pPlayers = startupData().players();

    pPlayerList_->deleteAllChildren();

    //  const NetNode::Ping& ping = MachLogNetwork::instance().node().pingInformation();

    for (size_t loop = 0; loop < 4; ++loop)
    {
        NETWORK_STREAM("player " << loop << " displayName " << pPlayers[loop].getDisplayName() << std::endl);
        double pingValue = 0;
        /*      for( NetNode::Ping::const_iterator i = ping.begin(); i != ping.end(); ++i )
        {
            if( (*i).first.nodeName() == pPlayers[loop].getDisplayName() )
                pingValue = (*i).second.lastProperPingTime_;
        }*/

        pPlayerListItem_[loop] = new MachGuiPlayerListItem(
            pStartupScreens_,
            pPlayerList_,
            240,
            pPlayers[loop].getDisplayName(),
            pPlayers[loop].race_,
            loop,
            pPlayers[loop].ready_,
            pPlayers[loop].host_,
            pingValue,
            pPlayers[loop].hasMachinesCD_);
    }

    pPlayerList_->childrenUpdated();

    // Next two lines must come in this order because displaySystemMessages() updates some data
    // that updateStartAndReadyButtons() is interested in
    displaySystemMessages();
    updateStartAndReadyButtons();
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiCtxImReady::createPlayerList DONE\n");
}

void MachGuiCtxImReady::updatePlayerList()
{
    NETWORK_STREAM("MachGuiCtxImReady::updatePlayerList this " << static_cast<const void*>(this) << "\n");
    NETWORK_INDENT(2);
    MachGuiStartupData::PlayerInfo* pPlayers = startupData().players();

    //  const NetNode::Ping& ping = MachLogNetwork::instance().node().pingInformation();

    for (size_t loop = 0; loop < 4; ++loop)
    {
        double pingValue = 0;
        /*      for( NetNode::Ping::const_iterator i = ping.begin(); i != ping.end(); ++i )
        {
            if( (*i).first.nodeName() == pPlayers[loop].getDisplayName() )
                pingValue = (*i).second.lastProperPingTime_;
        }*/

        pPlayerListItem_[loop]->updateInfo(
            pPlayers[loop].getDisplayName(),
            pPlayers[loop].race_,
            pPlayers[loop].ready_,
            pPlayers[loop].host_,
            pingValue,
            pPlayers[loop].hasMachinesCD_);
    }

    // Next two lines must come in this order because displaySystemMessages() updates some data
    // that updateStartAndReadyButtons() is interested in
    displaySystemMessages();
    updateStartAndReadyButtons();
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiCtxImReady::updatePlayerList DONE this " << static_cast<const void*>(this) << "\n");
}

void MachGuiCtxImReady::displaySystemMessages()
{
    if (! startupData().isHost())
    {
        bool included = startupData().includedInGame();
        if (included != startupData().updateIncludedInGame())
        {
            if (startupData().includedInGame())
            {
                // Display message saying host has added player into game
                GuiResourceString message(IDS_MENUMSG_HOSTADDED, startupData().getHostName());
                MachGuiChatWindow::addText(message.asString());
            }
            else
            {
                // Display message saying host has remove player from game
                GuiResourceString message(IDS_MENUMSG_HOSTREMOVED, startupData().getHostName());
                MachGuiChatWindow::addText(message.asString());
            }
        }
    }
}

void MachGuiCtxImReady::updateStartAndReadyButtons()
{
    // Update "I'm Ready" button
    if (startupData().isHost())
    {
        // Isn't used by host...
        pImReadyButton_->disabled(true);
    }
    else
    {
        // Grey out "I'm Ready" button if this player is not being considered for the game
        if (! startupData().includedInGame() || startupData().terminalMultiPlayerGameProblem())
        {
            pImReadyButton_->setReady(false);
            pImReadyButton_->disabled(true);
        }
        else
        {
            bool ready = startupData().isReady(startupData().playerName());
            pImReadyButton_->setReady(ready);
            pImReadyButton_->disabled(false);
        }
    }

    // Update start button
    pStartButton_->disabled(! startupData().canStartMultiPlayerGame());

    pSettingsButton_->disabled(startupData().terminalMultiPlayerGameProblem());
}

// virtual
void MachGuiCtxImReady::buttonEvent(MachGui::ButtonEvent buttonEvent)
{
    if (buttonEvent == MachGui::ButtonEvent::IMREADY)
    {
        ASSERT(! startupData().isHost(), "Host shouldn't be able to press I'm Ready button");

        pStartupScreens_->messageBroker().sendImReadyMessage(
            startupData().playerName(),
            ! startupData().isReady(startupData().playerName()));
    }
    else if (buttonEvent == MachGui::ButtonEvent::START)
    {
        ASSERT(startupData().isHost(), "Only host should be able to press Start button");

#ifndef DEMO
        if (startupData().doesAtLeastOnePlayerHaveMachinesCD()
#ifndef PRODUCTION
            || getenv("cb_dontcheckcd"))
#else
        )
#endif
        {
            // Assign colours to races that have been left as NORACE
            pStartupScreens_->startupData()->initMachLogNetwork();

            if (NetNetwork::instance().currentProtocol() != NetNetwork::ZONE)
                NetNetwork::instance().disableNewPlayers();

            //  Make sure all remote players have the correct game settings
            pStartupScreens_->startupData()->sendUpdatePlayersMessage();
            pStartupScreens_->startupData()->sendUpdateGameSettingsMessage();

            for (int i = 0; i < 3; ++i)
            {
                SysWindowsAPI::peekMessage();
                SysWindowsAPI::sleep(500);
                pStartupScreens_->messageBroker().sendStartMessage();
            }
            pStartupScreens_->switchContext(MachGuiStartupScreens::CTX_MULTI_GAME);
        }
        else
        {
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_MULTIPLAYERINSERTCD);
        }
#else // DEMO defined. \
    // Assign colours to races that have been left as NORACE
        pStartupScreens_->startupData()->initMachLogNetwork();

        if (NetNetwork::instance().currentProtocol() != NetNetwork::ZONE)
            NetNetwork::instance().disableNewPlayers();

        //  Make sure all remote players have the correct game settings
        pStartupScreens_->startupData()->sendUpdatePlayersMessage();
        pStartupScreens_->startupData()->sendUpdateGameSettingsMessage();

        for (int i = 0; i < 3; ++i)
        {
            SysWindowsAPI::peekMessage();
            SysWindowsAPI::sleep(500);
            pStartupScreens_->messageBroker().sendStartMessage();
        }
        pStartupScreens_->switchContext(MachGuiStartupScreens::CTX_MULTI_GAME);
#endif
    }
    else if (buttonEvent == MachGui::ButtonEvent::OK)
    {
        // Make sure there is a chat message worth sending ( i.e. not empty )
        if (pSingleLineEditBox_->text() != "" && ! pSingleLineEditBox_->clearTextOnNextChar())
        {
            // Add players name onto chat message
            string chatMessage(startupData().playerName());
            chatMessage += " : ";
            chatMessage += pSingleLineEditBox_->text();

            // Send message (if host hasn't terminated or another problem hasn't occured )
            if (! pStartupScreens_->startupData()->terminalMultiPlayerGameProblem())
            {
                pStartupScreens_->messageBroker().sendChatMessage(chatMessage);
            }

            // I won't get notified of my own chat message therefore I must put it
            // in the chat window manually
            pChatWindow_->addText(chatMessage);

            // Clear down chat message entry area ready for next message
            pSingleLineEditBox_->text("");
        }
    }
}

// virtual
bool MachGuiCtxImReady::okayToSwitchContext()
{
    NETWORK_STREAM("MachGuiCtxImReady::okayToSwitchContext this " << static_cast<const void*>(this) << "\n");
    NETWORK_INDENT(2);
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::EXIT)
    {
        NETWORK_STREAM("lastButtonEvent was EXIT so doing some stuff\n");
        if (MachLogNetwork::instance().isNetworkGame())
        {
            // if we got into this screen via a lobby session then we need to terminate correctly at this point.
            if (NetNetwork::instance().isLobbiedGame() && ! pStartupScreens_->ignoreHostLostSystemMessage())
            {
                pStartupScreens_->displayMsgBox(
                    IDS_MENUMSG_QUIT,
                    new MachGuiExitToInternetMessageBoxResponder(
                        pStartupScreens_,
                        MachGuiExitToInternetMessageBoxResponder::DO_NOT_UNLOAD_GAME)),
                    true;

                NETWORK_STREAM("lobbied game so returning false\n");
                NETWORK_INDENT(-2);
                return false;
            }

            if (startupData().isHost())
            {
                pStartupScreens_->messageBroker().sendHostCancelMessage();
            }
            else
            {
                pStartupScreens_->messageBroker().sendClientCancelMessage(startupData().playerName());
            }
            // Sleep to make sure that message gets sent
            for (int i = 0; i < 10; ++i)
            {
                // Transmit the termination messages!
                MachLogNetwork::instance().update();
                SysWindowsAPI::sleep(100);
                SysWindowsAPI::peekMessage();
            }
            // if the game is lobbied and the host cancelled then we won't be displaying a msgbox
            // but simply doing the work.
            if (NetNetwork::instance().isLobbiedGame())
            {
                MachLogNetwork::instance().terminateAndReset();
                pStartupScreens_->contextFinishFromLobby();
                return false;
            }
        }

        // Disconnect from network ( keep protocol!! )
        // Store connection type because terminateAndReset sets it to "".
        //
        // Hal 3/2/99. Commented out because it causes a crash. Unneccessary anyway?
        // string ct = startupData().connectionType();
        // MachLogNetwork::instance().terminateAndReset();
        // startupData().connectionType( ct );

        // Initialise all data in player info slots.
        startupData().resetPlayers();
    }
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiCtxImReady::okayToSwitchContext DONE this " << static_cast<const void*>(this) << "\n");
    return true;
}

MachGuiStartupData& MachGuiCtxImReady::startupData()
{
    PRE(pStartupScreens_);
    PRE(pStartupScreens_->startupData());

    return *pStartupScreens_->startupData();
}

void MachGuiCtxImReady::updateGameSettings()
{
    pReadOnlySettings_->deleteAllChildren();

    // Blit background to read only list box
    auto* shared = pStartupScreens_->getSharedBitmaps();
    auto backdrop = shared->getNamedBitmap("backdrop");
    shared->blitNamedBitmapFromArea(
        backdrop,
        pReadOnlySettings_->absoluteBoundary(),
        pReadOnlySettings_->absoluteBoundary().minCorner(),
        [shared, backdrop](const Gui::Box& box) {
            using namespace machgui::helper::menus;
            return centered_bitmap_transform(
                box,
                shared->getWidthOfNamedBitmap(backdrop),
                shared->getHeightOfNamedBitmap(backdrop));
        });

    int textWidth = (SETTINGS_MAXX - SETTINGS_MINX) * 0.66;
    int valueWidth = (SETTINGS_MAXX - SETTINGS_MINX) * 0.33;

    GuiResourceString mapSizeStr(IDS_MENULB_MAPSIZE);
    GuiResourceString terrainTypeStr(IDS_MENULB_TERRAINTYPE);
    GuiResourceString scenarioStr(IDS_MENULB_SCENARIO);
    GuiResourceString fogOfWarStr(IDS_MENU_FOGOFWAR);
    GuiResourceString resourcesStr(IDS_MENU_RESOURCES);
    GuiResourceString startingResourcesStr(IDS_MENU_STARTINGRESOURCES);
    GuiResourceString startingPositionStr(IDS_MENU_STARTPOS);
    GuiResourceString victoryConditionStr(IDS_MENU_VICTORYCONDITION);
    GuiResourceString techLevelStr(IDS_MENU_STARTINGTECHLEVEL);
    GuiResourceString broadcastAlliancesStr(IDS_MENU_BROADCAST_ALLIANCE);
    GuiResourceString disableFirstPersonStr(IDS_MENU_DISABLE_FIRST_PERSON);
    string whiteFont("gui/menu/smalwfnt.bmp");

    // Show read only game settings...
    new MachGuiText(pReadOnlySettings_, textWidth, fogOfWarStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().fogOfWarStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, resourcesStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().resourcesStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, startingResourcesStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().startingResourcesStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, startingPositionStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().startingPositionStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, techLevelStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().techLevelStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, mapSizeStr.asString());
    new MachGuiText(
        pReadOnlySettings_,
        valueWidth,
        startupData().scenario()->planet().system().menuString(),
        whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, broadcastAlliancesStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().broadcastAlliancesStr(), whiteFont);
    new MachGuiText(pReadOnlySettings_, textWidth, disableFirstPersonStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, startupData().disableFirstPersonStr(), whiteFont);

    // Following settings are split across two lines
    new MachGuiText(pReadOnlySettings_, textWidth, victoryConditionStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, "");
    new MachGuiText(pReadOnlySettings_, SETTINGS_MAXX - SETTINGS_MINX, " " + startupData().victoryConditionStr());
    new MachGuiText(pReadOnlySettings_, valueWidth, "");

    new MachGuiText(pReadOnlySettings_, textWidth, terrainTypeStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, "");
    new MachGuiText(
        pReadOnlySettings_,
        SETTINGS_MAXX - SETTINGS_MINX,
        " " + startupData().scenario()->planet().menuString());
    new MachGuiText(pReadOnlySettings_, valueWidth, "");

    new MachGuiText(pReadOnlySettings_, textWidth, scenarioStr.asString());
    new MachGuiText(pReadOnlySettings_, valueWidth, "");
    new MachGuiText(pReadOnlySettings_, SETTINGS_MAXX - SETTINGS_MINX, " " + startupData().scenario()->menuString());

    pReadOnlySettings_->childrenUpdated();
};

/* End CTXREADY.CPP *************************************************/
