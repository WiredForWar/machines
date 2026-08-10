/*
 * S R L M O D E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiSerialNetworkMode

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_SRLMODE_HPP
#define _MACHGUI_SRLMODE_HPP

#include "machgui/Network/NetworkProtocolMode.hpp"
#include "base/base.hpp"

class MachGuiDropDownListBoxCreator;

class GuiDisplayable;
class MachGuiStartupScreens;

class MachGuiSerialNetworkMode : public MachGuiNetworkProtocolMode
// Canonical form revoked
{
public:
    MachGuiSerialNetworkMode(GuiDisplayable*, MachGuiStartupScreens*);
    ~MachGuiSerialNetworkMode() override;

    void CLASS_INVARIANT;

    void setNetworkDetails() override;

    bool validNetworkDetails(bool isHost) override;

protected:
    void readNetworkDetails() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiSerialNetworkMode& t);

    MachGuiSerialNetworkMode(const MachGuiSerialNetworkMode&) = delete;
    MachGuiSerialNetworkMode& operator=(const MachGuiSerialNetworkMode&) = delete;

    MachGuiDropDownListBoxCreator* pComPortSelecter_{};
    MachGuiDropDownListBoxCreator* pBaudRateSelecter_{};
    MachGuiDropDownListBoxCreator* pParitySelecter_{};
    MachGuiDropDownListBoxCreator* pStopBitsSelecter_{};
    MachGuiDropDownListBoxCreator* pFlowSelecter_{};
};

#endif

/* End SRLMODE.HPP **************************************************/
