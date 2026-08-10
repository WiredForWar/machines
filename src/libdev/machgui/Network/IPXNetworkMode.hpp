/*
 * I P X M O D E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiIPXNetworkMode

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_IPXMODE_HPP
#define _MACHGUI_IPXMODE_HPP

#include "base/base.hpp"
#include "machgui/Network/NetworkProtocolMode.hpp"

class GuiDisplayable;
class MachGuiStartupScreens;

class MachGuiIPXNetworkMode : public MachGuiNetworkProtocolMode
// Canonical form revoked
{
public:
    MachGuiIPXNetworkMode(GuiDisplayable*, MachGuiStartupScreens*);
    ~MachGuiIPXNetworkMode() override;

    void CLASS_INVARIANT;

    void setNetworkDetails() override;

    bool validNetworkDetails(bool isHost) override;

protected:
    void readNetworkDetails() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiIPXNetworkMode& t);

    MachGuiIPXNetworkMode(const MachGuiIPXNetworkMode&) = delete;
    MachGuiIPXNetworkMode& operator=(const MachGuiIPXNetworkMode&) = delete;
};

#endif

/* End IPXMODE.HPP **************************************************/
