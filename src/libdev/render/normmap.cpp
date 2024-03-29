/*
 * N O R M M A P . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/normmap.hpp"

RenNormalMap::RenNormalMap()
{

    TEST_INVARIANT;
}

RenNormalMap::~RenNormalMap()
{
    TEST_INVARIANT;
}

void RenNormalMap::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const RenNormalMap& t)
{

    o << "RenNormalMap " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenNormalMap " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End NORMMAP.CPP **************************************************/
