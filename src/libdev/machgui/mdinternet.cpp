/*
 * I N E T M O D E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/mdinternet.hpp"
#include "stdlib/string.hpp"
#include "machgui/startup.hpp"
#include "gui/gui.hpp"
#include "gui/font.hpp"
#include "gui/manager.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/editbox.hpp"
#include "machgui/startup.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/strings.hpp"
#include "network/netnet.hpp"
#include "system/registry.hpp"

struct MachGuiInternetNetworkModeImpl
{
    MachGuiInternetNetworkModeImpl()
        : pIPAddressEntryBox_(nullptr)
    {
    }
    MachGuiSingleLineEditBox* pIPAddressEntryBox_;
};

MachGuiInternetNetworkMode::MachGuiInternetNetworkMode(GuiDisplayable* pParent, MachGuiStartupScreens* pStartupScreens)
    : MachGuiNetworkProtocolMode(pParent, pStartupScreens)
{
    pimpl_ = new MachGuiInternetNetworkModeImpl;

    readNetworkDetails();

    TEST_INVARIANT;
}

MachGuiInternetNetworkMode::~MachGuiInternetNetworkMode()
{
    TEST_INVARIANT;

    delete pimpl_;
}

void MachGuiInternetNetworkMode::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiInternetNetworkMode::setNetworkDetails()
{
    // This function will use the settings from the drop downs to configure network settings
    string ipAddress = pimpl_->pIPAddressEntryBox_->text();

    NetNetwork::instance().IPAddress(ipAddress);
    SysRegistry::instance().setStringValue("Network", "IP Address", ipAddress);
    NetNetwork::instance().initialiseConnection();
}

// virtual
bool MachGuiInternetNetworkMode::validNetworkDetails(bool /*isHost*/)
{
    return true;
}

// virtual
void MachGuiInternetNetworkMode::updateGUI()
{
    // Flash caret
    pimpl_->pIPAddressEntryBox_->update();
}

// virtual
void MachGuiInternetNetworkMode::charFocus()
{
    GuiManager::instance().charFocus(pimpl_->pIPAddressEntryBox_);
}

#define INM_MINX 62
#define INM_MINY 309
#define INM_WIDTH 110
#define INM_MAX_ADDRESS_NO 30

void MachGuiInternetNetworkMode::readNetworkDetails()
{
    GuiResourceString IPAddressHeading(IDS_MENU_IPADDRESS);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontWhite()));
    const int textHeight = font.charHeight() + 2;

    MachGuiMenuText* pIPAddressText = new MachGuiMenuText(
        &parent(),
        Gui::Box(Gui::Coord(INM_MINX, INM_MINY), font.textWidth(IPAddressHeading.asString()), textHeight),
        IDS_MENU_IPADDRESS,
        MachGui::Menu::smallFontLight());

    pimpl_->pIPAddressEntryBox_ = new MachGuiSingleLineEditBox(
        &startupScreens(),
        Gui::Box(Gui::Coord(INM_MINX, INM_MINY + textHeight), INM_WIDTH, textHeight + 4),
        font);
    pimpl_->pIPAddressEntryBox_->borderColour(MachGui::DARKSANDY());
    pimpl_->pIPAddressEntryBox_->border(true);
    std::string ipAddress = SysRegistry::instance().queryStringValue("Network", "IP Address");
    if (ipAddress.empty())
        ipAddress = "localhost";

    pimpl_->pIPAddressEntryBox_->text(ipAddress);

    charFocus();
}

std::ostream& operator<<(std::ostream& o, const MachGuiInternetNetworkMode& t)
{

    o << "MachGuiInternetNetworkMode " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiInternetNetworkMode " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End INETMODE.CPP *************************************************/
