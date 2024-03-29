/*
 * C O N T R O L I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/controli.hpp"

MachGuiControlPanelImpl::MachGuiControlPanelImpl()
{

    TEST_INVARIANT;
}

MachGuiControlPanelImpl::~MachGuiControlPanelImpl()
{
    TEST_INVARIANT;
}

void MachGuiControlPanelImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiControlPanelImpl& t)
{

    o << "MachGuiControlPanelImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiControlPanelImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End CONTROLI.CPP *************************************************/
