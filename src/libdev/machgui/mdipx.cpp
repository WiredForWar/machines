/*
 * I P X M O D E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/mdipx.hpp"
#include "network/netnet.hpp"

MachGuiIPXNetworkMode::MachGuiIPXNetworkMode(GuiDisplayable* pParent, MachGuiStartupScreens* pStartupScreens)
    : MachGuiNetworkProtocolMode(pParent, pStartupScreens)
{

    TEST_INVARIANT;
}

MachGuiIPXNetworkMode::~MachGuiIPXNetworkMode()
{
    TEST_INVARIANT;
}

void MachGuiIPXNetworkMode::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiIPXNetworkMode::setNetworkDetails()
{
    NetNetwork::instance().initialiseConnection();
}

// virtual
bool MachGuiIPXNetworkMode::validNetworkDetails(bool /*isHost*/)
{
    return true;
}

// virtual
void MachGuiIPXNetworkMode::readNetworkDetails()
{
}

std::ostream& operator<<(std::ostream& o, const MachGuiIPXNetworkMode& t)
{

    o << "MachGuiIPXNetworkMode " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiIPXNetworkMode " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End IPXMODE.CPP **************************************************/
