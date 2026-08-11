/*
 * M D E M M O D E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/Network/ModemNetworkMode.hpp"
#include "gui/gui.hpp"
#include "gui/Font.hpp"
#include "gui/Manager.hpp"
#include "machgui/DropDownListBoxCreator.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/SingleLineEditBox.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/strings.hpp"
#include "network/Network.hpp"
#include "machgui/MessageBoxResponder.hpp"

MachGuiModemNetworkMode::MachGuiModemNetworkMode(GuiDisplayable* pParent, MachGuiStartupScreens* pStartupScreens)
    : MachGuiNetworkProtocolMode(pParent, pStartupScreens)
{
    readNetworkDetails();

    TEST_INVARIANT;
}

MachGuiModemNetworkMode::~MachGuiModemNetworkMode()
{
    TEST_INVARIANT;
}

void MachGuiModemNetworkMode::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiModemNetworkMode::setNetworkDetails()
{
    // This function will use the settings from the drop downs to configure network settings
    std::string currentModem = pModemSelector_->currentText();
    std::string serverTelNo = pTelNoEntryBox_->text();

    //  NetNetwork::instance().phone( serverTelNo );
    //  NetNetwork::instance().modem( currentModem );
    NetNetwork::instance().initialiseConnection();
}

// virtual
bool MachGuiModemNetworkMode::validNetworkDetails(bool isHost)
{
    bool valid = true;

    if ((pTelNoEntryBox_->text() == "") && (! isHost))
    {
        startupScreens().displayMsgBox(IDS_MENUMSG_ENTERSERVERTELNO);
        valid = false;
    }
    return valid;
}

// virtual
void MachGuiModemNetworkMode::updateGUI()
{
    // calling the update method on the text entry box will cause its caret to flash
    pTelNoEntryBox_->update();
}

#define MNM_MINX 62
#define MNM_MINY 309
#define MNM_WIDTH 110
#define MNM_DEPTH 30
#define MNM_MINY2 MNM_MINY + MNM_DEPTH
#define MAX_TELNO_LEN 20

// virtual
void MachGuiModemNetworkMode::readNetworkDetails()
{
    GuiResourceString modemHeading(IDS_MENU_MODEMTYPE);
    GuiBmpFont font(Gui::getFont(MachGui::Menu::smallFontWhite()));
    const GuiBmpFont& headingFont = Gui::getFont(MachGui::Menu::smallFontLight());
    const int textHeight = font.height() + 2 * MachGui::menuScaleFactor();

    //  NetNetwork::Modems availableModems = NetNetwork::instance().modems();
    NetNetwork::Modems availableModems;

    //  ASSERT( availableModems.size(), "no modems");

    //  WAYNE_STREAM( "Modems" << std::endl << availableModems << std::endl );

    MachGuiMenuText* pModemText = new MachGuiMenuText(
        &parent(),
        Gui::Box(Gui::Coord(MNM_MINX, MNM_MINY), headingFont.horizontalAdvance(modemHeading.asString()), textHeight),
        IDS_MENU_MODEMTYPE,
        MachGui::Menu::smallFontLight());

    pModemSelector_ = new MachGuiDropDownListBoxCreator(
        &parent(),
        &startupScreens(),
        Gui::Coord(MNM_MINX, MNM_MINY + textHeight),
        MNM_WIDTH,
        true,
        true);
    pModemSelector_->setAvailText(availableModems);

    GuiResourceString telephoneNoHeading(IDS_MENU_TELNO);

    MachGuiMenuText* pTelephoneNoText = new MachGuiMenuText(
        &parent(),
        Gui::Box(Gui::Coord(MNM_MINX, MNM_MINY2), headingFont.horizontalAdvance(telephoneNoHeading.asString()), textHeight),
        IDS_MENU_TELNO,
        MachGui::Menu::smallFontLight());

    pTelNoEntryBox_ = new MachGuiSingleLineEditBox(
        &startupScreens(),
        Gui::Box(Gui::Coord(MNM_MINX, MNM_MINY2 + textHeight), MNM_WIDTH, textHeight + 4),
        font);
    pTelNoEntryBox_->ignoreSpaceAtBeginning(true);
    pTelNoEntryBox_->border(true);
    pTelNoEntryBox_->borderColour(MachGui::DARKSANDY());

    GuiManager::instance().charFocus(pTelNoEntryBox_);
}

std::ostream& operator<<(std::ostream& o, const MachGuiModemNetworkMode& t)
{
    o << "MachGuiModemNetworkMode " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiModemNetworkMode " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MDEMMODE.CPP *************************************************/
