/*
 * D I S P N O T I . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/dispnoti.hpp"

MachLogDispositionChangeNotifiable::MachLogDispositionChangeNotifiable()
{

    TEST_INVARIANT;
}

MachLogDispositionChangeNotifiable::~MachLogDispositionChangeNotifiable()
{
    TEST_INVARIANT;
}

void MachLogDispositionChangeNotifiable::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogDispositionChangeNotifiable& t)
{

    o << "MachLogDispositionChangeNotifiable " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogDispositionChangeNotifiable " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End DISPNOTI.CPP *************************************************/
