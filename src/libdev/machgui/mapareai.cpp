/*
 * M A P A R E A I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/mapareai.hpp"

MachGuiMapAreaImpl::MachGuiMapAreaImpl()
    : pMapAreaMagic_(nullptr)
{

    TEST_INVARIANT;
}

MachGuiMapAreaImpl::~MachGuiMapAreaImpl()
{
    TEST_INVARIANT;
}

void MachGuiMapAreaImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMapAreaImpl& t)
{

    o << "MachGuiMapAreaImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMapAreaImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MAPAREAI.CPP *************************************************/
