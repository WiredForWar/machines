/*
 * C T X M U L T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxmulti.hpp"

#include "base/diag.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/editbox.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/manager.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "gui/restring.hpp"
#include "network/session.hpp"
#include "network/netnet.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
class MachGuiProtocolListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    MachGuiProtocolListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        const string& text)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, text)
    {
    }

    ~MachGuiProtocolListBoxItem() override { }

    bool isSelectedProtocol()
    {
        // returns whether or not this item matches the currectly selected protocol
        // This will enable client code to reselect it as necessary
        if (strcasecmp(text().c_str(), startupScreens()->startupData()->connectionType().c_str()) == 0)
        {
            return true;
        }
        return false;
    }

protected:
    void select() override
    {
        DEBUG_STREAM(DIAG_NETWORK, "MachGuiProtocolListBoxItem::select()\n");
        MachGuiSingleSelectionListBoxItem::select();

        startupScreens()->startupData()->connectionType(text(), MachGuiStartupData::DO_NOT_INITIALISE_CONNECTION);
        DEBUG_STREAM(DIAG_NETWORK, "MachGuiProtocolListBoxItem::select() DONE\n");
    }

private:
    MachGuiProtocolListBoxItem(const MachGuiProtocolListBoxItem&);
    MachGuiProtocolListBoxItem& operator=(const MachGuiProtocolListBoxItem&);
};

//////////////////////////////////////////////////////////////////////////////////////////////

MachGuiCtxMultiplayer::MachGuiCtxMultiplayer(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sb", pStartupScreens)
{
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(56, 407, 246, 449) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::JOIN);
    MachGuiMenuButton* pExitBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(311, 407, 502, 449) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);
    pExitBtn->escapeControl(true);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    pSingleLineEditBox_ = new MachGuiSingleLineEditBox(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(106, 34) * MachGui::menuScaleFactor(),
            Gui::Size(338 * MachGui::menuScaleFactor(), font.charHeight())),
        font);
    pSingleLineEditBox_->maxChars(MAX_PLAYERNAME_LEN);

    if (pStartupScreens_->startupData()->playerName() == "")
    {
        GuiResourceString enterName(IDS_MENU_ENTERNAME);
        pSingleLineEditBox_->setText(enterName.asString());
        pSingleLineEditBox_->clearTextOnNextChar(true);
    }
    else
    {
        pSingleLineEditBox_->setText(pStartupScreens_->startupData()->playerName());
    }

    GuiManager::instance().charFocus(pSingleLineEditBox_);

    GuiResourceString connectionType(IDS_MENULB_CONNECTIONTYPE);

    size_t startY = 123 * MachGui::menuScaleFactor();
    int startX = 61 * MachGui::menuScaleFactor();
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(startX, startY),
            Gui::Size(font.textWidth(connectionType.asString()), font.charHeight() + 2 * MachGui::menuScaleFactor())),
        IDS_MENULB_CONNECTIONTYPE,
        MachGui::Menu::largeFontLight());
    startY += font.charHeight() + 3 * MachGui::menuScaleFactor();
    int listBoxWidth = 413 * MachGui::menuScaleFactor();
    MachGuiSingleSelectionListBox* pListBox = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            startX,
            startY,
            startX + listBoxWidth,
            287 * MachGui::menuScaleFactor()),
        1000 * MachGui::menuScaleFactor(),
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);

    // Get available protocols
    const NetNetwork::ProtocolMap& availableProtocols = NetNetwork::availableProtocols();

    MachGuiProtocolListBoxItem* pSelectedItem = nullptr;
    for (const NetNetwork::ProtocolSpec& protocol : availableProtocols)
    {
        MachGuiProtocolListBoxItem* pItem
            = new MachGuiProtocolListBoxItem(pStartupScreens, pListBox, listBoxWidth, protocol.first);
        if (pItem->isSelectedProtocol())
            pSelectedItem = pItem;
    }

    if (pSelectedItem)
        pSelectedItem->selectThisItem();

    pListBox->childrenUpdated();

    TEST_INVARIANT;
}

MachGuiCtxMultiplayer::~MachGuiCtxMultiplayer()
{
    TEST_INVARIANT;

    GuiManager::instance().charFocus(pStartupScreens_);
}

// virtual
bool MachGuiCtxMultiplayer::okayToSwitchContext()
{
    // Trim spaces off beginning of name
    string playerName = pSingleLineEditBox_->text();
    while (playerName.length() > 0 && playerName[0] == ' ')
    {
        playerName = &playerName[1];
    }

    // Was JOIN pressed?...
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::JOIN)
    {
        if (playerName == "" || // No name entered
            pSingleLineEditBox_->clearTextOnNextChar())
        {
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERNAME);

            return false;
        } // stricmp
        else if (
            strcasecmp(pSingleLineEditBox_->text().c_str(), MachGui::openText().c_str()) == 0
            || // Invalid name ( open/closed/computer reserved )
            strcasecmp(pSingleLineEditBox_->text().c_str(), MachGui::closedText().c_str()) == 0
            || strcasecmp(pSingleLineEditBox_->text().c_str(), MachGui::computerText().c_str()) == 0
            || strcasecmp(pSingleLineEditBox_->text().c_str(), MachGui::unavailableText().c_str()) == 0)
        {
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_INVALIDNAME);

            return false;
        }
        else if (pStartupScreens_->startupData()->connectionType().empty()) // No connection type specified
        {
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_CONNECTIONTYPE);

            return false;
        }
        else if (NetNetwork::instance().currentStatus() == NetNetwork::NETNET_CONNECTIONERROR)
        {
            uint errorId;

            switch (NetNetwork::instance().currentProtocol())
            {
                case NetNetwork::NetworkProtocol::MODEM:
                    errorId = IDS_MENUMSG_NOMODEM;
                    break;
                case NetNetwork::NetworkProtocol::IPX:
                    errorId = IDS_MENUMSG_NOIPX;
                    break;
                case NetNetwork::NetworkProtocol::TCPIP:
                    errorId = IDS_MENUMSG_NOTCPIP;
                    break;
                case NetNetwork::NetworkProtocol::SERIAL:
                    errorId = IDS_MENUMSG_NOSERIAL;
                    break;
                default:
                    errorId = IDS_MENUMSG_NETUNDEFINEDERROR;
                    break;
            }
            pStartupScreens_->displayMsgBox(errorId);

            return false;
        }
    }

    if (playerName != "" && ! pSingleLineEditBox_->clearTextOnNextChar())
    {
        pStartupScreens_->startupData()->playerName(playerName);
    }

    return true;
}

// virtual
void MachGuiCtxMultiplayer::update()
{
    pSingleLineEditBox_->update();
    animations_.update();
}

void MachGuiCtxMultiplayer::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxMultiplayer& t)
{

    o << "MachGuiCtxMultiplayer " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxMultiplayer " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End CTXMULTI.CPP *************************************************/
