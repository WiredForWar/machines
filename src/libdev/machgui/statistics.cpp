/*
 * S T A T I S T I C S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/statistics.hpp"

MachGuiStatistics::MachGuiStatistics()
{
    TEST_INVARIANT;
}

MachGuiStatistics::~MachGuiStatistics()
{
    TEST_INVARIANT;
}

void MachGuiStatistics::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiStatistics& t)
{

    o << "MachGuiStatistics " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiStatistics " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End STATISTICS.CPP ***********************************************/
