/*
 * M A C H D A T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/machdati.hpp"

#ifndef _INLINE
#include "machphys/internal/machdati.ipp"
#endif

MachPhysIMachineData::MachPhysIMachineData()
{

    TEST_INVARIANT;
}

MachPhysIMachineData::~MachPhysIMachineData()
{
    TEST_INVARIANT;
}

void MachPhysIMachineData::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysIMachineData& t)
{

    o << "MachPhysIMachineData " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysIMachineData " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MACHDATI.CPP *************************************************/
