/*
 * I N E T M O D E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiInternetNetworkMode

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_INETMODE_HPP
#define _MACHGUI_INETMODE_HPP

#include "base/base.hpp"
#include "machgui/Network/NetworkProtocolMode.hpp"

class MachGuiSingleLineEditBox;

class GuiDisplayable;
class MachGuiStartupScreens;

class MachGuiInternetNetworkMode : public MachGuiNetworkProtocolMode
// Canonical form revoked
{
public:
    MachGuiInternetNetworkMode(GuiDisplayable*, MachGuiStartupScreens*);
    ~MachGuiInternetNetworkMode() override;

    void CLASS_INVARIANT;

    void setNetworkDetails() override;
    void readNetworkDetails() override;

    bool validNetworkDetails(bool isHost) override;

    void updateGUI() override;

    void charFocus() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiInternetNetworkMode& t);

    MachGuiInternetNetworkMode(const MachGuiInternetNetworkMode&) = delete;
    MachGuiInternetNetworkMode& operator=(const MachGuiInternetNetworkMode&) = delete;

    MachGuiSingleLineEditBox* pIPAddressEntryBox_{};
};

#endif

/* End INETMODE.HPP *************************************************/
