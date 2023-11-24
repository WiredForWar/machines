/*
 * M C C O N V O I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/mcconvoi.hpp"

MachLogMachineConvoyImpl::MachLogMachineConvoyImpl()
{

    TEST_INVARIANT;
}

MachLogMachineConvoyImpl::~MachLogMachineConvoyImpl()
{
    TEST_INVARIANT;
}

void MachLogMachineConvoyImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogMachineConvoyImpl& t)
{

    o << "MachLogMachineConvoyImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogMachineConvoyImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MCCONVOI.CPP *************************************************/
