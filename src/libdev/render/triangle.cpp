/*
 * T R I A N G L E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/triangle.hpp"

RenTriangle::RenTriangle()
{

    TEST_INVARIANT;
}

RenTriangle::~RenTriangle()
{
    TEST_INVARIANT;
}

void RenTriangle::CLASS_INVARIANT
{
}

std::ostream& operator<<(std::ostream& o, const RenTriangle& t)
{

    o << "RenTriangle " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenTriangle " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End TRIANGLE.CPP *************************************************/
